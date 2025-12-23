#pragma once
#include "gates.h"
#include "helpers.h"

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
    Slot **D;      // input bus [N]
    Slot *LOAD;    // load signal
    TriStateGate *tsD;
} RegLoadPort;

typedef struct {
    Slot *Q;       // output bus [N]
    Slot *Q_not;   // optional
    Slot *EN;      // enable
    TriStateGate *tsQ;
    TriStateGate *tsQn;
} RegEnablePort;

typedef struct {
    int N;

    // Storage
    DLatch *bits;
    Node *internalBufferBus;

    // Clock
    Slot *CLK;

    // Ports
    int num_load_ports;
    int num_enable_ports;

    RegLoadPort *loadports;
    RegEnablePort  *enableports;
} NBitRegister;

void nreg_add_load_port(
    NBitRegister *r,
    int wp,
    Slot **D,
    Slot *LOAD
) {
    RegLoadPort *p = &r->loadports[wp];
    p->D = malloc(sizeof(Slot*) * r->N);
    p->LOAD = LOAD;

    p->tsD  = malloc(sizeof(TriStateGate) * r->N);

    for (int i = 0; i < r->N; i++) {
        tristate_init(&p->tsD[i],  D[i],  p->LOAD);
        p->D[i] = &p->tsD[i].out.resolved;
        node_add_slot(&r->internalBufferBus[i], p->D[i]);
    }
}

void nreg_add_enable_port(
    NBitRegister *r,
    int rp,
    Slot *Q,
    Slot *Q_not,
    Slot *EN
) {
    RegEnablePort *p = &r->enableports[rp];
    p->Q = Q;
    p->Q_not = Q_not;
    p->EN = EN;

    p->tsQ  = malloc(sizeof(TriStateGate) * r->N);
    p->tsQn = malloc(sizeof(TriStateGate) * r->N);

    for (int i = 0; i < r->N; i++) {
        tristate_init(&p->tsQ[i],  &r->bits[i].Q,     EN);
        tristate_init(&p->tsQn[i], &r->bits[i].Q_not, EN);

        Q[i]     = p->tsQ[i].out.resolved;
        Q_not[i] = p->tsQn[i].out.resolved;
    }
}

void nreg_init(
    NBitRegister *r,
    int N,
    int num_write_ports,
    int num_read_ports,
    Slot *CLK
) {
    r->N = N;
    r->CLK = CLK;

    r->num_load_ports = num_write_ports;
    r->num_enable_ports  = num_read_ports;

    r->bits = malloc(sizeof(DLatch) * N);

    r->internalBufferBus  = malloc(sizeof(Node) * N);
    allocate_node(r->internalBufferBus,  r->num_load_ports, N);

    r->loadports = malloc(sizeof(RegLoadPort) * num_write_ports);
    r->enableports = malloc(sizeof(RegEnablePort) * num_read_ports);

    // Initialize storage latches with dummy D/EN (patched later)
    for (int i = 0; i < N; i++) {
        r->bits[i].Q.value     = SIG_0;
        r->bits[i].Q_not.value = SIG_1;
        dlatch_init(&r->bits[i], &r->internalBufferBus[i].resolved, r->CLK);
    }
}

void nreg_eval(NBitRegister *r) {
    // Evaluate write enables
    for (int w = 0; w < r->num_load_ports; w++) {
        for (int i = 0; i < r->N; i++) {
            tristate_eval(&r->loadports[w].tsD[i]);
        }
    }

    // Evaluate storage
    for (int i = 0; i < r->N; i++) {
        node_resolve(&r->internalBufferBus[i]);
        dlatch_eval(&r->bits[i]);
    }

    // Evaluate read ports
    for (int rp = 0; rp < r->num_enable_ports; rp++) {
        RegEnablePort *p = &r->enableports[rp];
        for (int i = 0; i < r->N; i++) {
            tristate_eval(&p->tsQ[i]);
            tristate_eval(&p->tsQn[i]);
            p->Q[i]     = p->tsQ[i].out.resolved;
            p->Q_not[i] = p->tsQn[i].out.resolved;
        }
    }
}
