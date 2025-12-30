#pragma once

#include "transistor.h"
#include "regfile.h"
#include "regalu.h"
#include "register.h"
#include "helpers.h"
#include "rcl.h"
#include "clock.h"
#include "timing.h"

// ================= CPU =================

typedef struct {
    int N;
    ClockGen clkGen;

    Slot *dummy;
    Slot **one;
    Slot **zero;

    Node *dataBus;
    Node *stackBus;
    Node *addressLBus;
    Node *addressHBus;

    RegFile rf;
    RegALU alu;
    ProgramCounter pc;

    RCL rcl; // Random Control Logic
    TGL tgl; // Timing Generation Logic

    DecodeRom *decRom; 

    NBitRegister ABL; // Address Bus Low Register
    NBitRegister ABH; // Address Bus High Register
    NBitRegister PD; // Predecode Register
    NBitRegister IR; // Instruction Register
    NBitRegister DOR; // Data Output Register

    // TEST
    Slot **IR_IN;
    Slot *IR_OUT;
    Slot *TGL_OUT;

} CPU;

// ================= API =================

void cpu_init(CPU *cpu, int N, Slot *CLK, Slot **one, Slot **zero, Slot *dummy);
void multi_eval(CPU *cpu, Slot *CLK);

