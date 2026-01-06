#include "faketgl.h"

/* ========================================================= */
/* Internal storage */

static Slot t0, t1x, t2, t3, t4, t5, t6;
static Slot sync;

/* ========================================================= */

static void clear_all(void)
{
    t0.value   = SIG_0;
    sync.value = SIG_0;
    t1x.value  = SIG_0;
    t2.value   = SIG_0;
    t3.value   = SIG_0;
    t4.value   = SIG_0;
    t5.value   = SIG_0;
    t6.value   = SIG_0;

}

/* ========================================================= */

void tgl_init(FAKETGL *tgl, Slot *CLK)
{
    tgl->CLK = CLK;

    tgl->notT0  = &t0;
    tgl->SYNC = &sync;
    tgl->notT1X = &t1x;
    tgl->notT2  = &t2;
    tgl->notT3  = &t3;
    tgl->notT4  = &t4;
    tgl->notT5  = &t5;
    tgl->notT6  = &t6;


    tgl->out[0] = tgl->notT0;
    tgl->out[1] = tgl->notT1X;
    tgl->out[2] = tgl->notT2;
    tgl->out[3] = tgl->notT3;
    tgl->out[4] = tgl->notT4;
    tgl->out[5] = tgl->notT5;
    tgl->out[6] = tgl->notT6;

    tgl->cycle   = 7;
    tgl->prev_clk.value = SIG_0;

    clear_all();
    t0.value = SIG_1;
}

/* ========================================================= */

void tgl_reset(FAKETGL *tgl)
{
    tgl->cycle = 0;
    clear_all();
    t0.value = SIG_1;
}

/* ========================================================= */
/* Call every simulation step */

void tgl_eval(FAKETGL *tgl)
{
    int clk = tgl->CLK->value;

    /* detect rising edge */
    if (tgl->prev_clk.value == SIG_0 && clk == SIG_1) {

        clear_all();

        tgl->cycle++;
        if (tgl->cycle > 7)
            tgl->cycle = 0;

        switch (tgl->cycle) {
            case 0: t0.value  = SIG_1; break;
            case 1: sync.value = SIG_1; break;
            case 2: t1x.value = SIG_1; break;
            case 3: t2.value  = SIG_1; break;
            case 4: t3.value  = SIG_1; break;
            case 5: t4.value  = SIG_1; break;
            case 6: t5.value  = SIG_1; break;
            case 7: t6.value  = SIG_1; break;
        }
    }

    tgl->prev_clk.value = clk;
}
