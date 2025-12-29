#include <stdlib.h>

#include "cpu.h"


// ================= CPU =================

static void bus_drive(Slot *in, const int *v, int N) {
    for (int i = 0; i < N; i++)
        in[i].value = v[i];
}

static void bus_release(Slot *in, int N) {
    for (int i = 0; i < N; i++)
        in[i].value = SIG_Z;
}

void multi_eval (RegFile *rf, ProgramCounter *pc, RegALU *alu) {
    printf("    CLOCK 0   \n");
    rf->CLK->value = SIG_0;
    for (int i=0; i < 10; i++)
        regfile_eval(rf, pc, alu);    
    //dump(rf, pc, alu);
    printf("    CLOCK 1   \n");
    rf->CLK->value = SIG_1;
    for (int i=0; i < 10; i++)
        regfile_eval(rf, pc, alu);
    //dump(rf, pc, alu);
    printf("    CLOCK 0   \n");
    rf->CLK->value = SIG_0;
    for (int i=0; i < 10; i++)
        regfile_eval(rf, pc, alu);    
    //dump(rf, pc, alu);

}

/* ---------------- CPU ---------------- */

void cpu_init(
    CPU *cpu,
    int N, 
    Slot *CLK, 
    Slot **one, 
    Slot **zero,
    Slot *dummy
    )
{
    cpu->CLK = CLK;
    cpu->N = N;
    cpu->one = one;
    cpu->zero = zero;
    cpu->dummy = dummy;

    /* -------- Allocate buses -------- */
    cpu->dataBus = malloc(sizeof(Node) * N);
    cpu->stackBus = malloc(sizeof(Node) * N);
    cpu->addressLBus = malloc(sizeof(Node) * N);
    cpu->addressHBus = malloc(sizeof(Node) * N);

    allocate_node(cpu->dataBus,  1, N);
    allocate_node(cpu->stackBus,  1, N);
    allocate_node(cpu->addressLBus, 1, N);
    allocate_node(cpu->addressHBus, 1, N);

    rcl_init(&cpu->rcl);

    /* -------- Init regfile -------- */
    regfile_init(
        &cpu->rf, 
        cpu->N, 
        cpu->CLK, 
        cpu->one, 
        cpu->zero, 
        cpu->dummy, 
        cpu->dataBus, 
        cpu->stackBus,
        cpu->addressLBus,
        cpu->addressHBus, 
        cpu->rcl);
    
    pc_init(
        &cpu->pc, 
        cpu->N, 
        cpu->CLK, 
        cpu->one,
        cpu->zero,
        cpu->dummy, 
        &cpu->rf.regPCLS, 
        &cpu->rf.regPCHS, 
        &cpu->rf.regPCL, 
        &cpu->rf.regPCH,
        cpu->one[0], 
        cpu->zero[0], 
        cpu->rcl
    );
    
    regfile_connect2buses(&cpu->rf);

    alu_init(
        &cpu->alu, 
        N, 
        &CLK, 
        one, 
        zero, 
        dummy, 
        &cpu->rf.regA, 
        &cpu->rf.regB, 
        &cpu->rf.regAH, 
        cpu->one[0], 
        cpu->zero[0]
    );

}
