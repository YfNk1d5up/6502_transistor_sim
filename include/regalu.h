#pragma once
#include "register.h"
#include "alu.h"

typedef struct {
    int N;
    Slot *CLK;

    // Registers
    NBitRegister *regA;
    NBitRegister *regB;
    NBitRegister *regAH;

    // Storage
    Slot *A;
    Slot *B;

    // Combinational core
    ALUNBit core;

} RegALU;

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
    );

void alu_eval(RegALU *alu);
