#include "register.h"
#include "transistor.h"

// =========== Timing Generation Logic ===========

typedef struct {
    Slot *CLK;

    DLatch t0;
    DLatch t1;
    DLatch t1x;
    DLatch t2;
    DLatch t3;
    DLatch t4;
    DLatch t5;
    DLatch t6;

    Slot *SYNC; //T1

    Slot *notT0;
    Slot *notT1X;
    Slot *notT2;
    Slot *notT3;
    Slot *notT4;
    Slot *notT5;
    Slot *notT6;

    Slot *out[7];
} TGL;

void timing_init(TGL *tgl, Slot *CLK);

void timing_eval(TGL *tgl);