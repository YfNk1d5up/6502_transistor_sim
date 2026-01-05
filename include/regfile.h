#pragma once
#include "gates.h"
#include "register.h"
#include "pc.h"
#include "regalu.h"
#include "rcl.h"
#include <stdlib.h>

// --- 6502 Register File ---

typedef struct {
    int N; 
    Slot *CLK;      // shared clock
    Slot **one;
    Slot **zero;
    Slot *dummy; // dummy slot for unused ports

    Slot **dataBusD;   // Data bus
    Node *dataBus;

    NOTGate *notG; // Data bus Inverted
    TriStateGate *tsNot;
    Slot **notDataBusD;
    Node *notDataBus;


    Slot **stackBusD;  // Stack Pointer bus
    Node *stackBus;

    Slot **addressLBusD; // Memory Address Low bus
    Node *addressLBus;

    Slot **addressHBusD; // Memory Address High bus
    Node *addressHBus;

    // Registers
    NBitRegister regDL; // Input Data Latch as register
    NBitRegister regA; // ALU A
    NBitRegister regB; // ALU B
    NBitRegister regAH; // ALU Adder Hold Register
    NBitRegister regAC;   // Accumulator
    NBitRegister regX;   // Index X
    NBitRegister regY;   // Index Y
    NBitRegister regSP;  // Stack Pointer
    NBitRegister regP;   // Processor Status
    NBitRegister regPCLS; // Program Counter Select low 
    NBitRegister regPCHS; // Program Counter Select high
    NBitRegister regPCL; // Program Counter low
    NBitRegister regPCH; // Program Counter high

    // Registers Outputs
    Slot *DL_DB;
    Slot *DL_ADL;
    Slot *DL_ADH;
    Slot *AH06_SB; // bits 0-6
    Slot *AH7_SB; //bit 7
    Slot *AH_ADL;
    Slot *AC_DB;
    Slot *AC_SB;  
    Slot *X_SB;  
    Slot *Y_SB;  
    Slot *S_SB; 
    Slot *S_ADL;
    Slot *P_DB;  
    Slot *PCL_DB;
    Slot *PCL_ADL;
    Slot *PCH_DB;
    Slot *PCH_ADH; 
} RegFile;

void regfile_init(
    RegFile *rf,
    int N,
    Slot *CLK,
    Slot **one,
    Slot **zero,
    Slot *dummy,
    Node *dataBus,
    Node *stackBus,
    Node *addressLBus,
    Node *addressHBus,
    RCL rcl
);

void regfile_connect2buses(RegFile *rf);

void regfile_eval(RegFile *rf, ProgramCounter *pc, RegALU *alu);