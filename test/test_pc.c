#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../pc.h"
#include "../register.h"
#include "../gates.h"
#include "../helpers.h"

static void print_pc(Slot *busL, Slot *busH, int N)
{
    unsigned value = 0;
    
    for (int i = 0; i < N; i++) {
        if (busL[i].value == SIG_1)
            value |= (1 << i);
        if (busH[i].value == SIG_1)
            value |= (1 << (i + N));
    }

    printf("PC = 0x%04X\n", value);
}

int main(void)
{
    const int N = 8;

    /* --------------------
       Clock & controls
       -------------------- */
    Slot CLK    = { SIG_0 };
    Slot *LOAD_ADL_PCL = malloc(sizeof(Slot));
    Slot *LOAD_ADH_PCH = malloc(sizeof(Slot));
    Slot *EN_PCL_DB = malloc(sizeof(Slot));
    Slot *EN_PCL_ADL = malloc(sizeof(Slot));
    Slot *EN_PCH_DB = malloc(sizeof(Slot));
    Slot *EN_PCH_ADH = malloc(sizeof(Slot));
    Slot *LOAD_PCL_PCL = malloc(sizeof(Slot));
    Slot *LOAD_PCH_PCH = malloc(sizeof(Slot));
    Slot *EN_I_PC = malloc(sizeof(Slot));
    LOAD_ADL_PCL->value = SIG_0;
    LOAD_ADH_PCH->value = SIG_0;
    EN_PCL_DB->value = SIG_0;
    EN_PCL_ADL->value = SIG_0;
    EN_PCH_DB->value = SIG_0;
    EN_PCH_ADH->value = SIG_0;
    LOAD_PCL_PCL->value = SIG_0;
    LOAD_PCH_PCH->value = SIG_0;
    EN_I_PC->value = SIG_0;

    Slot *one;
    one = malloc(sizeof(Slot));
    one->value = SIG_1;

    Slot *zero;
    zero = malloc(sizeof(Slot));
    zero->value = SIG_0;

    Slot *dummy;
    dummy = malloc(sizeof(Slot) * N);
    for (int i = 0; i < N; i++) {
        dummy[i].value = SIG_Z;
    }



    /* --------------------
       Register buses
       -------------------- */

    Slot *addressLBusD[8];
    for (int i = 0; i < 8; i++) {
        addressLBusD[i] = malloc(sizeof(Slot));
        addressLBusD[i]->value = SIG_Z;
    }
    Slot *addressHBusD[8];
    for (int i = 0; i < 8; i++) {
        addressHBusD[i] = malloc(sizeof(Slot));
        addressHBusD[i]->value = SIG_Z;
    }
    Slot PCL_DB[8] = {0,0,0,0,0,0,0,0};
    Slot PCL_ADL[8] = {0,0,0,0,0,0,0,0};
    Slot PCH_DB[8] = {0,0,0,0,0,0,0,0};
    Slot PCH_ADH[8] = {0,0,0,0,0,0,0,0};

    /* --------------------
       Registers
       -------------------- */

    NBitRegister PCLS;
    NBitRegister PCHS;
    NBitRegister PCL;
    NBitRegister PCH;

    nreg_init(&PCLS, N, 2, 1, &CLK);
    nreg_add_load_port(&PCLS, 0, addressLBusD, LOAD_ADL_PCL);
    // second one loads from internal PC low bus in PC implementation
 

    nreg_init(&PCHS, N, 2, 1, &CLK);
    nreg_add_load_port(&PCHS, 0, addressHBusD, LOAD_ADH_PCH);
    // second one loads from internal PC high bus in PC implementation

    nreg_init(&PCL, N, 1, 3, &CLK);
    // load from internal PC logic in PC implementation
    nreg_add_enable_port(&PCL, 0, PCL_DB, dummy, EN_PCL_DB);
    nreg_add_enable_port(&PCL, 1, PCL_ADL, dummy, EN_PCL_ADL);
    // third one to internal PC low BUS increment in PC implementation

    nreg_init(&PCH, N, 1, 3, &CLK);
    // load from internal PC logic in PC implementation
    nreg_add_enable_port(&PCH, 0, PCH_DB, dummy, EN_PCH_DB);
    nreg_add_enable_port(&PCH, 1, PCH_ADH, dummy, EN_PCH_ADH);
    // third one to internal PC low BUS increment in PC implementation



    /* --------------------
       Program Counter
       -------------------- */
    ProgramCounter pc;
    pc_init(&pc, 
        N, 
        &CLK, 
        one, 
        zero, 
        dummy, 
        &PCLS, 
        &PCHS,
        &PCL, 
        &PCH,
        LOAD_PCL_PCL,
        LOAD_PCH_PCH,
        EN_I_PC
    );


    /* Connect PC outputs to register inputs */
    //PCL.D = pc.L;
    //PCH.D = pc.H;

    /* --------------------
       Test sequence
       -------------------- */
    printf("Initial state:\n");

    print_slots("PCLSBusQ", pc.PCLSBusQ, N);
    print_slots("PCHSBusQ", pc.PCHSBusQ, N);
    print_slots("PCLBusQ", pc.PCLBusQ, N);
    print_slots_ptr("PCLBusD", pc.PCLBusD, N);
    print_slots("PCHBusQ", pc.PCHBusQ, N);
    print_slots_ptr("PCHBusD", pc.PCHBusD, N);
    print_slots("PCL_DB", PCL_DB, N);
    print_slots("PCL_ADL", PCL_ADL, N);
    print_slots("PCH_DB", PCH_DB, N);
    print_slots("PCH_ADH", PCH_ADH, N);

    EN_PCL_DB->value = SIG_0;
    EN_PCH_DB->value = SIG_0;
    EN_PCL_ADL->value = SIG_0;
    EN_PCH_ADH->value = SIG_0;
    printf("AddressBus output:\n");
    print_pc(PCL_ADL, PCH_ADH, N);

    nreg_eval(&PCLS);
    nreg_eval(&PCHS);
    pc_eval(&pc);
    nreg_eval(&PCL);
    nreg_eval(&PCH);

    printf("DataBus output:\n");
    print_pc(PCL_DB, PCH_DB, N);
    printf("AddressBus output:\n");
    print_pc(PCL_ADL, PCH_ADH, N);

    EN_PCL_DB->value = SIG_1;
    EN_PCH_DB->value = SIG_1;

    nreg_eval(&PCLS);
    nreg_eval(&PCHS);
    pc_eval(&pc);
    nreg_eval(&PCL);
    nreg_eval(&PCH);

    printf("DataBus output:\n");
    print_pc(PCL_DB, PCH_DB, N);
    printf("AddressBus output:\n");
    print_pc(PCL_ADL, PCH_ADH, N);

    EN_PCL_DB->value = SIG_0;
    EN_PCH_DB->value = SIG_0;
    EN_PCL_ADL->value = SIG_1;
    EN_PCH_ADH->value = SIG_1;

    nreg_eval(&PCLS);
    nreg_eval(&PCHS);
    pc_eval(&pc);
    nreg_eval(&PCL);
    nreg_eval(&PCH);

    printf("DataBus output:\n");
    print_pc(PCL_DB, PCH_DB, N);
    printf("AddressBus output:\n");
    print_pc(PCL_ADL, PCH_ADH, N);

    EN_PCL_ADL->value = SIG_0;
    EN_PCH_ADH->value = SIG_0;

    EN_I_PC->value = SIG_1;
    LOAD_PCL_PCL->value = SIG_1;
    LOAD_PCH_PCH->value = SIG_1;

    for (int cycle = 0; cycle < pow(2, 16) ; cycle++) {// cycle < pow(2, 16) - 1
        /* Rising edge */
        CLK.value = SIG_1;

        nreg_eval(&PCLS);
        nreg_eval(&PCHS);
        pc_eval(&pc);
        nreg_eval(&PCL);
        nreg_eval(&PCH);
        
        /* Falling edge */
        CLK.value = SIG_0;

        EN_PCL_DB->value = SIG_1;
        EN_PCH_DB->value = SIG_1;
        EN_PCL_ADL->value = SIG_0;
        EN_PCH_ADH->value = SIG_0;

        nreg_eval(&PCLS);
        nreg_eval(&PCHS);
        pc_eval(&pc);
        nreg_eval(&PCL);
        nreg_eval(&PCH);

        printf("DataBus output:\n");
        print_pc(PCL_DB, PCH_DB, N);
        printf("AddressBus output:\n");
        print_pc(PCL_ADL, PCH_ADH, N);

    }

    /* Pause PC */
    EN_I_PC->value = SIG_0;

    printf("\nPC paused:\n");
    for (int i = 0; i < 3; i++) {
        CLK.value = SIG_1;
        EN_PCL_ADL->value = SIG_1;
        EN_PCH_ADH->value = SIG_1;

        nreg_eval(&PCLS);
        nreg_eval(&PCHS);
        pc_eval(&pc);
        nreg_eval(&PCL);
        nreg_eval(&PCH);
        CLK.value = SIG_0;

        nreg_eval(&PCLS);
        nreg_eval(&PCHS);
        pc_eval(&pc);
        nreg_eval(&PCL);
        nreg_eval(&PCH);

        printf("DataBus output:\n");
        print_pc(PCL_DB, PCH_DB, N);
        printf("AddressBus output:\n");
        print_pc(PCL_ADL, PCH_ADH, N);
    }

    return 0;
}
