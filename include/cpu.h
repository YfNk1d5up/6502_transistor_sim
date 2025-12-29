#pragma once

#include "transistor.h"
#include "regfile.h"
#include "regalu.h"
#include "register.h"
#include "helpers.h"
#include "rcl.h"

// ================= CPU =================

typedef struct {
    int N;
    Slot *CLK;

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

    RCL rcl;

    NBitRegister ABL;
    NBitRegister ABH;
    NBitRegister IR;
    NBitRegister PS;
    NBitRegister DOR;

} CPU;

// ================= API =================

void cpu_init(CPU *cpu, int N, Slot *CLK, Slot **one, Slot **zero, Slot *dummy);
void multi_eval(RegFile *rf, ProgramCounter *pc, RegALU *alu);

