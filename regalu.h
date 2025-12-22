#include <stdlib.h>
#include "transistor.h"
#include "register.h"
#include "alu.h"

typedef struct {
    int N;
    Slot *CLK;
    Slot **inputD;      // input bus (resolved)
    Slot **outputQ;      // output bus

    // Registers
    NBitRegister regA;
    NBitRegister regB;

    // Storage
    Slot *A;
    Slot *B;

    // Combinational core
    ALUNBit core;

    // Tri-state
    TriStateGate *tsOUT;
} RegALU;

typedef struct {
    Slot *EN_A;
    Slot *EN_B;
    Slot *EN_OUT;
} RegAluEn;

void alu_init(RegALU *alu, int N, Slot *CLK, Slot **inputD, RegAluEn *en)
{
    alu->N = N;
    alu->CLK = CLK;
    alu->inputD = inputD;

    alu->A   = malloc(sizeof(Slot) * N);
    alu->B   = malloc(sizeof(Slot) * N);

    for (int i = 0; i < N; i++) {
        alu->A[i].value   = SIG_0;
        alu->B[i].value   = SIG_0;
    }

    // Registers
    nreg_init(&alu->regA,   N, alu->inputD, alu->A,   en->EN_A);
    nreg_init(&alu->regB,   N, alu->inputD, alu->B,   en->EN_B);

    // Core ALU uses register outputs
    alu_nbit_init(&alu->core, N, alu->A, alu->B);

    // Tri-state output
    alu->tsOUT  = malloc(sizeof(TriStateGate) * N);
    alu->outputQ = malloc(sizeof(Slot*) * N);

    for (int i = 0; i < N; i++) {
        tristate_init(&alu->tsOUT[i], alu->core.result[i], en->EN_OUT);
        alu->outputQ[i] = &alu->tsOUT[i].out.resolved;
    }
    
}

void alu_eval(RegALU *alu)
{
    // Registers
    nreg_eval(&alu->regA);
    nreg_eval(&alu->regB);

    // Combinational
    alu_nbit_eval(&alu->core);
    
    // Output enable
    for (int i = 0; i < alu->N; i++) {
        tristate_eval(&alu->tsOUT[i]);
    }
}
