#pragma once
#include "transistor.h"
#include "decoderom.h"
#include <stdint.h>

#define RCL_BITS_COUNT 62
// ================= Random Control Logic =================

typedef struct {
    Slot ctlSlots[RCL_BITS_COUNT];

    // Input Data Latch
    Slot *EN_DL_DB;
    Slot *EN_DL_ADL;
    Slot *EN_DL_ADH;

    // Open drain ADH
    Slot *EN_0_ADH0;
    Slot *EN_0_ADH17;

    // Address buses
    Slot *LOAD_ADH_ABH;
    Slot *LOAD_ADL_ABL;

    // PC
    Slot *LOAD_PCL_PCL;
    Slot *LOAD_ADL_PCL;
    Slot *EN_I_PC;
    Slot *EN_PCL_DB;
    Slot *EN_PCL_ADL;

    Slot *LOAD_PCH_PCH;
    Slot *LOAD_ADH_PCH;
    Slot *EN_PCH_DB;
    Slot *EN_PCH_ADH;

    // Pass MOSFET
    Slot *EN_SB_ADH;
    Slot *EN_SB_DB;

    // Open drain ADL
    Slot *EN_0_ADL0;
    Slot *EN_0_ADL1;
    Slot *EN_0_ADL2;

    // Stack Pointer
    Slot *EN_S_ADL;
    Slot *LOAD_SB_S;
    Slot *HOLD_S_S;
    Slot *EN_S_SB;

    // ALU
    Slot *LOAD_notDB_ADD;
    Slot *LOAD_DB_ADD;
    Slot *LOAD_ADL_ADD;

    Slot *EN_I_AD0C;
    Slot *EN_DAA;
    Slot *EN_DSA;

    Slot *OP_SUMS;
    Slot *OP_ANDS;
    Slot *OP_EORS;
    Slot *OP_ORS;
    Slot *OP_SRS;

    Slot *EN_ADD_ADL;
    Slot *EN_ADD06_SB;
    Slot *EN_ADD7_SB;

    Slot *LOAD_0_ADD;
    Slot *LOAD_SB_ADD;

    // Accumulator
    Slot *LOAD_SB_AC;
    Slot *EN_AC_DB;
    Slot *EN_AC_SB;

    // X
    Slot *LOAD_SB_X;
    Slot *EN_X_SB;

    // Y
    Slot *LOAD_SB_Y;
    Slot *EN_Y_SB;

    // Processor Status
    Slot *EN_P_DB;

    Slot *LOAD_DB0_P_C;
    Slot *LOAD_IR5_P_C;
    Slot *LOAD_ACR_P_C;

    Slot *LOAD_DB1_P_Z;
    Slot *LOAD_DBZ_P_Z;

    Slot *LOAD_DB2_P_I;
    Slot *LOAD_IR5_P_I;

    Slot *LOAD_DB3_P_D;
    Slot *LOAD_IR5_P_D;

    Slot *LOAD_DB6_P_V;
    Slot *LOAD_AVR_P_V;
    Slot *LOAD_I_P_V;

    Slot *LOAD_DB7_P_N;
} RCL;

void rcl_init(RCL *rcl);

void rcl_apply(RCL *rcl, uint64_t word);

void rcl_eval(
    RCL *rcl,
    uint16_t key
);
