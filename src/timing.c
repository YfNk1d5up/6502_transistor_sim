#include "timing.h"

// =========== Timing Generation Logic ===========

void timing_init(TGL *tgl, Slot *CLK) {
    tgl->t0.Q.value = SIG_1;
    tgl->t1.Q.value = SIG_1;
    tgl->t1x.Q.value= SIG_1;
    tgl->t2.Q.value = SIG_1;
    tgl->t3.Q.value = SIG_1;
    tgl->t4.Q.value = SIG_1;
    tgl->t5.Q.value = SIG_1;
    tgl->t6.Q.value = SIG_0;

    tgl->notT0 = &tgl->t0.Q;
    tgl->SYNC = &tgl->t1.Q;
    tgl->notT1X = &tgl->t1x.Q;
    tgl->notT2 = &tgl->t2.Q;
    tgl->notT3 = &tgl->t3.Q;
    tgl->notT4 = &tgl->t4.Q;
    tgl->notT5 = &tgl->t5.Q;
    tgl->notT6 = &tgl->t6.Q;

    dlatch_init(&tgl->t0,  tgl->notT6, CLK);
    dlatch_init(&tgl->t1,  tgl->notT0, CLK);
    dlatch_init(&tgl->t1x, tgl->SYNC, CLK);
    dlatch_init(&tgl->t2,  tgl->notT1X, CLK);
    dlatch_init(&tgl->t3,  tgl->notT2, CLK);
    dlatch_init(&tgl->t4,  tgl->notT3, CLK);
    dlatch_init(&tgl->t5,  tgl->notT4, CLK);
    dlatch_init(&tgl->t6,  tgl->notT5, CLK);

    tgl->out[0] = tgl->notT0;
    tgl->out[1] = tgl->notT1X;
    tgl->out[2] = tgl->notT2;
    tgl->out[3] = tgl->notT3;
    tgl->out[4] = tgl->notT4;
    tgl->out[5] = tgl->notT5;
    tgl->out[6] = tgl->notT6;
                
}

void timing_eval(TGL *tgl) {
    dlatch_eval(&tgl->t0);
    dlatch_eval(&tgl->t1);
    dlatch_eval(&tgl->t1x);
    dlatch_eval(&tgl->t2);
    dlatch_eval(&tgl->t3);
    dlatch_eval(&tgl->t4);
    dlatch_eval(&tgl->t5);
    dlatch_eval(&tgl->t6);
}