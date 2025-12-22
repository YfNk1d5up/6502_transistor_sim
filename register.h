#pragma once
#include "gates.h"

// -------- D LATCH --------

typedef struct {
    Slot *D;
    Slot *EN;

    Slot D_not;

    Slot S;
    Slot R;

    Slot Q;
    Slot Q_not;

    NOTGate not_d;
    ANDGate and_s;
    ANDGate and_r;
    NORGate nor_q;
    NORGate nor_qn;
} DLatch;

void dlatch_init(DLatch *l, Slot *D, Slot *EN) {
    l->D = D;
    l->EN = EN;

    l->Q.value = SIG_0;
    l->Q_not.value = SIG_1;

    not_init(&l->not_d, D);

    and_init(&l->and_s, D, EN);
    and_init(&l->and_r, &l->D_not, EN);

    nor_init(&l->nor_q, &l->R, &l->Q_not);
    nor_init(&l->nor_qn, &l->S, &l->Q);
}

void dlatch_eval(DLatch *l) {
    if (l->D->value == SIG_Z) {
        return;
    } // don't like it either 

    not_eval(&l->not_d);
    l->D_not = l->not_d.out.resolved;

    and_eval(&l->and_s);
    and_eval(&l->and_r);

    l->S = l->and_s.out.resolved;
    l->R = l->and_r.out.resolved;

    // Cross-coupled NOR latch — iterate to stabilize
    Slot Q_prev = l->Q;
    Slot Qn_prev = l->Q_not;
    const int max_iter = 3; // 2-3 iterations usually enough

    for (int i = 0; i < max_iter; i++) {
        nor_eval(&l->nor_q);
        nor_eval(&l->nor_qn);

        l->Q = l->nor_q.out.resolved;
        l->Q_not = l->nor_qn.out.resolved;

        // Stop early if stabilized (i don't like it)
        if (l->Q.value == Q_prev.value && l->Q_not.value == Qn_prev.value)
            break;

        Q_prev = l->Q;
        Qn_prev = l->Q_not;
    }
}

// --- DFlipFlop ---
/*
typedef struct {
    Slot *D;
    Slot *CLK;

    Slot *Q;
    Slot *Q_not;

    DLatch dlatch;

    NOTGate not_q;
    NOTGate not_edge;
    ANDGate and_edge;
} DFlipFlop;

void dff_init(DFlipFlop *ff, Slot *D, Slot *CLK) {
    ff->D = D;
    ff->CLK = CLK;
    ff->q.value = SIG_0;
    ff->q_not.value = SIG_1;
    not_init(&ff->not_q, &ff->q);
    not_init(&ff->not_edge, &ff->CLK);
    and_init(&ff->and_edge, &ff->CLK, &ff->not_edge.out.resolved);
    dlatch_init(&ff->dlatch, D, &ff->and_edge.out.resolved)
}

void dff1_eval(DFlipFlop1Bit *ff) {
    not_eval(&ff->not_edge);
    and_eval(&ff->and_edge);
    dlatch_eval(&ff->dlatch);
    ff->q.value = &ff->dlatch->q.out.resolved;
    not_eval(&ff->not_q);
    ff->q_not = &ff->not_q.out.resolved;
}

// --- in simulation DLatch is used against DFlipFlop as
//     CLK and not CLK will always be false
*/

// --- N-bit Register ---

typedef struct {
    int N;

    Slot **D;     // input bus [width]
    Slot *REG;  // internal output
    Slot *Q;     // output bus [width]
    Slot *Q_not; // optional

    Slot *CLK;    // shared enable (clock)

    DLatch *bits;

    // Controls
    Slot *LOAD_Reg;
    Slot *EN_Reg;
    ANDGate andLoadReg;

    // Output Enable logic 
    ANDGate *andEnReg;
    TriStateGate *tsRegQ;
    TriStateGate *tsRegQNot;
} NBitRegister;

void nreg_init(NBitRegister *r, int N, Slot **D, Slot *Q, Slot *LOAD, Slot *EN, Slot *CLK) {
    r->N = N;
    r->D = D;
    r->Q = Q;
    r->LOAD_Reg = LOAD;
    r->EN_Reg = EN;
    r->CLK = CLK;

    r->bits = malloc(sizeof(DLatch) * N);
    r->Q_not = malloc(sizeof(Slot*));

    and_init(&r->andLoadReg, r->LOAD_Reg, r->CLK);

    for (int i = 0; i < N; i++) {
        dlatch_init(&r->bits[i], D[i], &r->andLoadReg.out.resolved);
    }

    r->tsRegQ   = malloc(sizeof(TriStateGate) * N);
    r->tsRegQNot   = malloc(sizeof(TriStateGate) * N);
    for (int i = 0; i < N; i++) {
        tristate_init(&r->tsRegQ[i], &r->bits[i].Q, r->EN_Reg);
        tristate_init(&r->tsRegQNot[i], &r->bits[i].Q_not, r->EN_Reg);
        r->Q[i] = r->tsRegQ[i].out.resolved;
        r->Q_not[i] = r->tsRegQNot[i].out.resolved;
    }
}

void nreg_eval(NBitRegister *r) {
    and_eval(&r->andLoadReg);
    for (int i = 0; i < r->N; i++) {
        dlatch_eval(&r->bits[i]);
    }

    // Commit outputs
    for (int i = 0; i < r->N; i++) {
        tristate_eval(&r->tsRegQ[i]);
        tristate_eval(&r->tsRegQNot[i]);
        r->Q[i] = r->tsRegQ[i].out.resolved;
        r->Q_not[i] = r->tsRegQNot[i].out.resolved;
    }
}
