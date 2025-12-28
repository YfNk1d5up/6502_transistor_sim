#pragma once
#include "fulladder.h"
#include "register.h"
#include "gates.h"
#include "helpers.h"
#include <stdlib.h>

// --- Program Counter ---

typedef struct {
    int N;              // bits per register (8)

    Slot *CLK;
    Slot prev_clk;
    NOTGate not_clk_rising_edge;
    ANDGate and_clk_rising_edge;

    Slot *LOAD_PCL_PCL;
    Slot *LOAD_PCH_PCH;
    Slot *EN_I_PC;

    Slot **PCLBusD; // Internal Program Counter Low bus
    Slot **PCHBusD; // Internal Program Counter High bus
    Slot *PCLSBusQ; // Internal Program Counter Select Low to increment logic
    Slot *PCHSBusQ; // Internal Program Counter Select High to increment logic 
    Slot *PCLBusQ; 
    Slot *PCHBusQ; 

    // External registers (from regfile)
    NBitRegister *PCLS;
    NBitRegister *PCHS;
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
    Slot **one,
    Slot **zero,
    Slot *dummy,
    NBitRegister *PCLS,
    NBitRegister *PCHS,
    NBitRegister *PCL,
    NBitRegister *PCH,
    Slot *one_ctl,
    Slot *zero_ctl,
    Slot *LOAD_PCL_PCL,
    Slot *LOAD_PCH_PCH,
    Slot *EN_I_PC
) {
    pc->N     = N;

    pc->CLK   = CLK;
    pc->prev_clk.value = SIG_0;
    not_init(&pc->not_clk_rising_edge, &pc->prev_clk);
    and_init(&pc->and_clk_rising_edge, pc->CLK, &pc->not_clk_rising_edge.out.resolved); 

    pc->LOAD_PCL_PCL = LOAD_PCL_PCL;
    pc->LOAD_PCH_PCH = LOAD_PCH_PCH;
    pc->EN_I_PC = EN_I_PC;

    pc->PCLS = PCLS;
    pc->PCHS = PCHS;
    pc->PCL   = PCL;
    pc->PCH   = PCH;

    pc->PCLBusD = malloc(sizeof(Slot*) * N);
    pc->PCHBusD = malloc(sizeof(Slot*) * N);
    pc->PCLSBusQ = malloc(sizeof(Slot) * N);
    pc->PCHSBusQ = malloc(sizeof(Slot) * N);
    pc->PCLBusQ = malloc(sizeof(Slot) * N);
    pc->PCHBusQ = malloc(sizeof(Slot) * N);
    for (int i = 0; i < N; i++) {
        pc->PCLBusQ[i].value = SIG_Z;
        pc->PCLBusD[i] = &pc->PCLBusQ[i];

        pc->PCHBusQ[i].value = SIG_Z;
        pc->PCHBusD[i] = &pc->PCHBusQ[i];

        pc->PCLSBusQ[i].value = SIG_Z;
        pc->PCHSBusQ[i].value = SIG_Z;
    }

    pc->incL = malloc(sizeof(Slot) * N);
    pc->incH = malloc(sizeof(Slot) * N);
    pc->L    = malloc(sizeof(Slot*) * N);
    pc->H    = malloc(sizeof(Slot*) * N);
    pc->PCL_Q = malloc(sizeof(Slot) * N);
    pc->PCH_Q = malloc(sizeof(Slot) * N);

    for (int i = 0; i < N; i++) {
        pc->incL[i].value = SIG_0;
        pc->incH[i].value = SIG_0;
        pc->PCL_Q[i].value = SIG_0;
        pc->PCH_Q[i].value = SIG_0;
    }

    // Clock enable gate
    and_init(&pc->clk_gate, &pc->and_clk_rising_edge.out.resolved, pc->EN_I_PC);
    // --- Incrementers ---
    // PCL + 1
    for (int i = 1; i < N; i++) {
        pc->incL[i].value = SIG_0;
        pc->incH[i].value = SIG_0;
    }
    pc->incL[0] = pc->clk_gate.out.resolved;
    full_adder_nbit_init(&pc->addL, pc->PCLSBusQ, pc->incL, N);

    // PCH + carry from PCL
    and_init(&pc->carry_gate, pc->addL.cout, &pc->clk_gate.out.resolved);

    pc->incH[0] = pc->carry_gate.out.resolved;

    full_adder_nbit_init(&pc->addH, pc->PCHSBusQ, pc->incH, N);

    // Registers load incremented value
    pc->L = pc->addL.sum;
    pc->H = pc->addH.sum;

    nreg_add_load_port(pc->PCLS, 1, pc->PCLBusD, pc->LOAD_PCL_PCL);
    nreg_add_enable_port(pc->PCLS, 0, pc->PCLSBusQ, dummy, one_ctl);
    nreg_add_load_port(pc->PCL, 0, pc->L, one_ctl);
    nreg_add_enable_port(pc->PCL, 2, pc->PCLBusQ, dummy, one_ctl);

    nreg_add_load_port(pc->PCHS, 1, pc->PCHBusD, pc->LOAD_PCH_PCH);
    nreg_add_enable_port(pc->PCHS, 0, pc->PCHSBusQ, dummy, one_ctl);
    nreg_add_load_port(pc->PCH, 0, pc->H, one_ctl);
    nreg_add_enable_port(pc->PCH, 2, pc->PCHBusQ, dummy, one_ctl);

}

void pc_eval(ProgramCounter *pc)
{
    // Clock enable

    for (int i = 1; i < pc->N; i++) {
        pc->incL[i].value = SIG_0;
        pc->incH[i].value = SIG_0;
    }
    // --- LOW BYTE ---
    not_eval(&pc->not_clk_rising_edge);
    and_eval(&pc->and_clk_rising_edge); 
    and_eval(&pc->clk_gate);
    pc->incL[0] = pc->clk_gate.out.resolved;
    full_adder_nbit_eval(&pc->addL);
    // --- HIGH BYTE ---
    and_eval(&pc->carry_gate);
    pc->incH[0] = pc->carry_gate.out.resolved;
    full_adder_nbit_eval(&pc->addH);
    pc->prev_clk.value = pc->CLK->value;
}
