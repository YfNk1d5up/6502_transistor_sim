#pragma once

#include "fulladder.h"
#include "register.h"
#include "gates.h"
#include <stdlib.h>

// --- Program Counter ---

typedef struct {
    int N;              // bits per register (8)
    Slot *CLK;
    Slot *PC_EN;

    // External registers (from regfile)
    NBitRegister *PCL;
    NBitRegister *PCH;

    // Clock gating
    ANDGate clk_gate;
    ANDGate carry_gate;

    // Increment logic
    FullAdderNBit addL;     // PCL + 1
    FullAdderNBit addH;     // PCH + carry

    // Internal buses for incremented values
    Slot **L;
    Slot **H;

    // Increment buses
    Slot *incL;
    Slot *incH;

    Slot *PCL_Q;
    Slot *PCH_Q;

} ProgramCounter;

void pc_init(
    ProgramCounter *pc,
    int N,
    Slot *CLK,
    Slot *PC_EN,
    NBitRegister *PCL,
    NBitRegister *PCH
) {
    pc->N     = N;
    pc->CLK   = CLK;
    pc->PC_EN = PC_EN;
    pc->PCL   = PCL;
    pc->PCH   = PCH;

    pc->incL = malloc(sizeof(Slot) * N);
    pc->incH = malloc(sizeof(Slot) * N);
    pc->L    = malloc(sizeof(Slot*) * N);
    pc->H    = malloc(sizeof(Slot*) * N);
    pc->PCL_Q = malloc(sizeof(Slot) * N);
    pc->PCH_Q = malloc(sizeof(Slot) * N);

    for (int i = 0; i < N; i++) {
        pc->L[i] = malloc(sizeof(Slot));
        pc->H[i] = malloc(sizeof(Slot));
        pc->L[i]->value = SIG_0;
        pc->H[i]->value = SIG_0;
        pc->incL[i].value = SIG_0;
        pc->incH[i].value = SIG_0;
        pc->PCL_Q[i].value = SIG_0;
        pc->PCH_Q[i].value = SIG_0;
    }

    nreg_init(pc->PCL, N, pc->L, pc->PCL_Q, pc->CLK);
    nreg_init(pc->PCH, N, pc->H, pc->PCH_Q, pc->CLK);

    // Clock enable gate
    and_init(&pc->clk_gate, CLK, PC_EN);
    // --- Incrementers ---
    // PCL + 1
    for (int i = 1; i < N; i++) {
        pc->incL[i].value = SIG_0;
        pc->incH[i].value = SIG_0;
    }
    pc->incL[0] = pc->clk_gate.out.resolved;
    full_adder_nbit_init(&pc->addL, PCL->Q, pc->incL, N);

    // PCH + carry from PCL
    and_init(&pc->carry_gate, pc->addL.cout, &pc->clk_gate.out.resolved);

    pc->incH[0] = pc->carry_gate.out.resolved;

    full_adder_nbit_init(&pc->addH, PCH->Q, pc->incH, N);

    // Registers load incremented value
    for (int i = 0; i < N; i++) {
        pc->L[i]->value = pc->addL.sum[i]->value;
        pc->H[i]->value = pc->addH.sum[i]->value;
    }

}

void pc_eval(ProgramCounter *pc)
{
    // Clock enable
    and_eval(&pc->clk_gate);
    and_eval(&pc->carry_gate);
    for (int i = 1; i < pc->N; i++) {
        pc->incL[i].value = SIG_0;
        pc->incH[i].value = SIG_0;
    }
    pc->incL[0] = pc->clk_gate.out.resolved;
    pc->incH[0] = pc->carry_gate.out.resolved;

    // --- LOW BYTE ---
    full_adder_nbit_eval(&pc->addL);
    // --- HIGH BYTE ---
    full_adder_nbit_eval(&pc->addH);

    // Registers load incremented value
    for (int i = 0; i < pc->N; i++) {
        pc->L[i]->value = pc->addL.sum[i]->value;
        pc->H[i]->value = pc->addH.sum[i]->value;
    }
}
