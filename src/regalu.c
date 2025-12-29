#include <stdlib.h>
#include "regalu.h"


void alu_init(
    RegALU *alu, 
    int N, 
    Slot *CLK, 
    Slot **one,
    Slot **zero,
    Slot *dummy,
    NBitRegister *regA,
    NBitRegister *regB,
    NBitRegister *regAH,
    Slot *one_ctl,
    Slot *zero_ctl
    // opcodes
    )
{
    alu->N = N;
    alu->CLK = CLK;
    alu->regA = regA;
    alu->regB = regB;
    alu->regAH = regAH;

    alu->A   = malloc(sizeof(Slot) * N);
    alu->B   = malloc(sizeof(Slot) * N);

    for (int i = 0; i < N; i++) {
        alu->A[i].value   = SIG_Z;
        alu->B[i].value   = SIG_Z;
    }

    // Registers ports
    nreg_add_enable_port(alu->regA, 0, alu->A, dummy, one_ctl);
    nreg_add_enable_port(alu->regB, 0, alu->B, dummy, one_ctl);
    
    // Core ALU uses register outputs
    alu_nbit_init(&alu->core, N, alu->A, alu->B);

    nreg_add_load_port(alu->regAH, 0, alu->core.result, one_ctl);
    
}

void alu_eval(RegALU *alu)
{
    // Combinational
    alu_nbit_eval(&alu->core);
}
