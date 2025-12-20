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
    Slot *Q;     // output bus [width]
    Slot *Q_not; // optional

    Slot *CLK;    // shared enable (clock)

    DLatch *bits;
} NBitRegister;

void nreg_init(NBitRegister *r, int N, Slot **D, Slot *Q, Slot *CLK) {
    r->N = N;
    r->D = D;
    r->Q = Q;
    r->CLK = CLK;

    r->bits = malloc(sizeof(DLatch) * N);
    r->Q_not = malloc(sizeof(Slot*));

    for (int i = 0; i < N; i++) {
        dlatch_init(&r->bits[i], D[i], CLK);

        r->Q[i] = r->bits[i].Q;
    }
}

void nreg_eval(NBitRegister *r) {
    for (int i = 0; i < r->N; i++) {
        dlatch_eval(&r->bits[i]);
    }

    // Commit outputs (optional but clean)
    for (int i = 0; i < r->N; i++) {
        r->Q[i].value = r->bits[i].Q.value;
        r->Q_not[i].value = r->bits[i].Q_not.value;
    }
}
