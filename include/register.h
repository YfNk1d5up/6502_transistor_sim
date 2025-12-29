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

void dlatch_init(DLatch *l, Slot *D, Slot *EN);

void dlatch_eval(DLatch *l);
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
    Slot prev_clk;
    NOTGate not_clk_rising_edge;
    ANDGate and_clk_rising_edge;

    // Ports
    int num_load_ports;
    int num_enable_ports;

    RegLoadPort *loadports;
    RegEnablePort  *enableports;
} NBitRegister;

void nreg_add_load_port(
    NBitRegister *r,
    int lp,
    Slot **D,
    Slot *LOAD
);

void nreg_add_enable_port(
    NBitRegister *r,
    int ep,
    Slot *Q,
    Slot *Q_not,
    Slot *EN
);

void nreg_init(
    NBitRegister *r,
    int N,
    int num_load_ports,
    int num_enable_ports,
    Slot *CLK
);

void nreg_eval(NBitRegister *r);
