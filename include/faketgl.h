#pragma once
#include "transistor.h"


/* ================= Timing Generator ================= */

typedef struct {
    
    Slot *CLK;   /* clock input */
    Slot prev_clk;
    Slot *SYNC; //T1

    Slot *notT0;
    Slot *notT1X;
    Slot *notT2;
    Slot *notT3;
    Slot *notT4;
    Slot *notT5;
    Slot *notT6;

    Slot *out[7];
    int cycle;  /* current cycle index (0..6) */
} FAKETGL;

/* ================= API ================= */

void tgl_init(FAKETGL *tgl, Slot *CLK);
void tgl_reset(FAKETGL *tgl);
void tgl_eval(FAKETGL *tgl);