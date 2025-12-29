#pragma once
#include "fulladder.h"
#include "register.h"
#include "rcl.h"
#include "gates.h"

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
    RCL rcl
);
void pc_eval(ProgramCounter *pc);