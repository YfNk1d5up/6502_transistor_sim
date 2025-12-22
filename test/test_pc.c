#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../pc.h"
#include "../register.h"
#include "../gates.h"

static void print_pc(NBitRegister PCH, NBitRegister PCL, int N)
{
    unsigned value = 0;
    
    for (int i = 0; i < N; i++) {
        if (PCL.Q[i].value == SIG_1)
            value |= (1 << i);
        if (PCH.Q[i].value == SIG_1)
            value |= (1 << (i + N));
    }

    printf("PC = 0x%04X\n", value);
}

int main(void)
{
    const int N = 8;

    /* --------------------
       Clock & enable
       -------------------- */
    Slot CLK    = { SIG_0 };
    Slot PC_EN  = { SIG_0 };

    /* --------------------
       Register buses
       -------------------- */
    Slot inputDL[8] = {0,0,0,0,0,0,0,0};
    Slot inputDH[8] = {0,0,0,0,0,0,0,0};
    Slot PCL_Q[8] = {0,0,0,0,0,0,0,0};
    Slot PCH_Q[8] = {0,0,0,0,0,0,0,0};

    Slot *PCL_D[8];
    for (int i = 0; i < 8; i++)
        PCL_D[i] = &inputDL[i];

    Slot *PCH_D[8];
    for (int i = 0; i < 8; i++)
        PCH_D[i] = &inputDH[i];

    /* --------------------
       Registers
       -------------------- */
    NBitRegister PCL;
    NBitRegister PCH;

    /* --------------------
       Program Counter
       -------------------- */
    ProgramCounter pc;
    pc_init(&pc, N, &CLK, &PC_EN, &PCL, &PCH);

    /* Connect PC outputs to register inputs */
    //PCL.D = pc.L;
    //PCH.D = pc.H;

    /* --------------------
       Test sequence
       -------------------- */
    printf("Initial state:\n");
    print_pc(PCH, PCL, N);

    PC_EN.value = SIG_1;

    for (int cycle = 0; cycle < pow(2, 16) - 1 ; cycle++) {
        /* Rising edge */
        CLK.value = SIG_1;

        pc_eval(&pc);
        nreg_eval(&PCL);
        nreg_eval(&PCH);
        
        /* Falling edge */
        CLK.value = SIG_0;

        printf("After cycle %2d: ", cycle + 1);
        print_pc(PCH, PCL, N);
    }

    /* Pause PC */
    PC_EN.value = SIG_0;

    printf("\nPC paused:\n");
    for (int i = 0; i < 3; i++) {
        CLK.value = SIG_1;
        pc_eval(&pc);
        nreg_eval(&PCL);
        nreg_eval(&PCH);
        CLK.value = SIG_0;

        print_pc(PCH, PCL, N);
    }

    return 0;
}
