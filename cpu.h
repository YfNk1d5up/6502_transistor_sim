#include "gates.h"
#include "alu.h"
#include "register.h"

// --- N-bit CPU ---

typedef struct {
    int N; // width of registers and ALU

    // Registers
    RegisterNBit regA;
    RegisterNBit regB;
    RegisterNBit regOut;

    // ALU
    ALUNBit alu;

    // Buses
    Slot *busA;   // output of regA
    Slot *busB;   // output of regB
    Slot *aluOut; // ALU output

    // Control signals (Slot so we can use gates)
    Slot selA;
    Slot selB;
    Slot weOut;
    Slot op_add;
    Slot op_and;
    Slot op_or;
    Slot op_xor;
    Slot clk;
} CPU_NBit;

void cpu_nbit_init(CPU_NBit *cpu, int N) {
    cpu->N = N;

    // Allocate buses
    cpu->busA   = malloc(sizeof(Slot) * N);
    cpu->busB   = malloc(sizeof(Slot) * N);
    cpu->aluOut = malloc(sizeof(Slot) * N);

    // Control signals default to 0
    cpu->selA.value   = SIG_1; // active for now
    cpu->selB.value   = SIG_1;
    cpu->weOut.value  = SIG_0;
    cpu->op_add.value = SIG_0;
    cpu->op_and.value = SIG_0;
    cpu->op_or.value  = SIG_0;
    cpu->op_xor.value = SIG_0;
    cpu->clk.value    = SIG_0;

    // Initialize N-bit registers
    register_nbit_init(&cpu->regA, N, cpu->busA, cpu->regA.Q, &cpu->clk);
    register_nbit_init(&cpu->regB, N, cpu->busB, cpu->regB.Q, &cpu->clk);
    register_nbit_init(&cpu->regOut, N, cpu->aluOut, cpu->regOut.Q, &cpu->clk);

    // Initialize N-bit ALU
    alu_nbit_init(&cpu->alu, N, cpu->regA.Q, cpu->regB.Q);

    // Connect ALU control pins
    cpu->alu.op_add  = cpu->op_add;
    cpu->alu.op_and  = cpu->op_and;
    cpu->alu.op_or   = cpu->op_or;
    cpu->alu.op_xor  = cpu->op_xor;
}

void cpu_nbit_eval(CPU_NBit *cpu) {
    // --- Read registers onto ALU buses using sel signals ---
    for (int i = 0; i < cpu->N; i++) {
        Slot tmpA, tmpB;

        // busA[i] = regA[i] AND selA
        ANDGate andA;
        and_init(&andA, &cpu->regA.Q[i], &cpu->selA);
        and_eval(&andA);
        cpu->busA[i] = andA.out.resolved;

        // busB[i] = regB[i] AND selB
        ANDGate andB;
        and_init(&andB, &cpu->regB.Q[i], &cpu->selB);
        and_eval(&andB);
        cpu->busB[i] = andB.out.resolved;
    }

    // --- ALU operation ---
    alu_nbit_eval(&cpu->alu);

    // --- Write ALU output to regOut masked by write-enable ---
    for (int i = 0; i < cpu->N; i++) {
        ANDGate weMask;
        and_init(&weMask, &cpu->alu.result[i], &cpu->weOut);
        and_eval(&weMask);
        cpu->regOut.D[i] = weMask.out.resolved;
    }

    // --- Update register outputs on clock ---
    register_nbit_eval(&cpu->regOut);
}
