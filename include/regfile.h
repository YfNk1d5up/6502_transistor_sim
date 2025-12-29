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

typedef struct {
    Slot *LOAD_SB_ADD; // ALU A
    Slot *LOAD_0_ADD;

    Slot *LOAD_DB_ADD; // ALU B
    Slot *LOAD_notDB_ADD;
    Slot *LOAD_ADL_ADD;

    Slot *LOAD_SB_AC;

    Slot *LOAD_SB_X;

    Slot *LOAD_SB_Y;

    Slot *LOAD_SB_S;

    Slot *LOAD_DB_P;

    Slot *LOAD_ADL_PCL;
    Slot *LOAD_PCL_PCL;

    Slot *LOAD_ADH_PCH;
    Slot *LOAD_PCH_PCH;

    Slot *EN_ADD_ADL; // ALU AHR
    Slot *EN_ADD06_SB; // bits 0-6 of AHR
    Slot *EN_ADD7_SB; // bit 7
    
    Slot *EN_AC_DB;
    Slot *EN_AC_SB;

    Slot *EN_X_SB;

    Slot *EN_Y_SB;

    Slot *EN_S_SB;
    Slot *EN_S_ADL;

    Slot *EN_P_DB;

    Slot *EN_PCL_DB;
    Slot *EN_PCL_ADL;

    Slot *EN_PCH_DB;
    Slot *EN_PCH_ADH;

    Slot *EN_I_PC;

    Slot *one;
    Slot *zero;
} RegFileEn;

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