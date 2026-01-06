#pragma once

#include "transistor.h"
#include "regfile.h"
#include "regalu.h"
#include "register.h"
#include "helpers.h"
#include "rcl.h"
#include "clock.h"
#include "faketgl.h"
#include "fakeram.h"

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

    Node *extDataBus;
    Slot **extDataBusD;

    Slot *outDataBus;
    Slot *outAddressBusL;
    Slot *outAddressBusH;
    Slot **outAddressBus;

    RegFile rf;
    RegALU alu;
    ProgramCounter pc;

    RCL rcl; // Random Control Logic
    FAKETGL tgl; // Timing Generation Logic

    DecodeRom *decRom; 

    NBitRegister ABL; // Address Bus Low Register
    NBitRegister ABH; // Address Bus High Register
    NBitRegister PD; // Predecode Register
    NBitRegister IR; // Instruction Register
    NBitRegister DOR; // Data Output Register
    NOTGate *enOutBuffers_not;
    ANDGate *enOutBuffers_and;
    TriStateGate *extDataBusTristate;

    Slot *RnotW;

    Slot *internalPD_Q;
    Slot ** internalIR_D;
    Slot *internalIR_Q;
    Slot **internalRCL_D;

} CPU;

// ================= API =================

void cpu_init(CPU *cpu, int N, Slot *CLK, Slot **one, Slot **zero, Slot *dummy);
void multi_eval(CPU *cpu, Slot *CLK, FAKERAM *ram);

