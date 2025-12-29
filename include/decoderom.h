#include <stdint.h>
#include <stdlib.h>

enum {
    RCL_EN_DL_DB = 0,
    RCL_EN_DL_ADL,
    RCL_EN_DL_ADH,

    // Open drain ADH
    RCL_EN_0_ADH0,
    RCL_EN_0_ADH17,

    // Address buses
    RCL_LOAD_ADH_ABH,
    RCL_LOAD_ADL_ABL,

    // PC
    RCL_LOAD_PCL_PCL,
    RCL_LOAD_ADL_PCL,
    RCL_EN_I_PC,
    RCL_EN_PCL_DB,
    RCL_EN_PCL_ADL,

    RCL_LOAD_PCH_PCH,
    RCL_LOAD_ADH_PCH,
    RCL_EN_PCH_DB,
    RCL_EN_PCH_ADH,

    // Pass MOSFET
    RCL_EN_SB_ADH,
    RCL_EN_SB_DB,

    // Open drain ADL
    RCL_EN_0_ADL0,
    RCL_EN_0_ADL1,
    RCL_EN_0_ADL2,

    // Stack Pointer
    RCL_EN_S_ADL,
    RCL_LOAD_SB_S,
    RCL_HOLD_S_S,
    RCL_EN_S_SB,

    // ALU
    RCL_LOAD_notDB_ADD,
    RCL_LOAD_DB_ADD,
    RCL_LOAD_ADL_ADD,

    RCL_EN_I_AD0C,
    RCL_EN_DAA,
    RCL_EN_DSA,

    RCL_OP_SUMS,
    RCL_OP_ANDS,
    RCL_OP_EORS,
    RCL_OP_ORS,
    RCL_OP_SRS,

    RCL_EN_ADD_ADL,
    RCL_EN_ADD06_SB,
    RCL_EN_ADD7_SB,

    RCL_LOAD_0_ADD,
    RCL_LOAD_SB_ADD,

    // Accumulator
    RCL_LOAD_SB_AC,
    RCL_EN_AC_DB,
    RCL_EN_AC_SB,

    // X
    RCL_LOAD_SB_X,
    RCL_EN_X_SB,

    // Y
    RCL_LOAD_SB_Y,
    RCL_EN_Y_SB,

    // Processor Status
    RCL_EN_P_DB,

    RCL_LOAD_DB0_P_C,
    RCL_LOAD_IR5_P_C,
    RCL_LOAD_ACR_P_C,

    RCL_LOAD_DB1_P_Z,
    RCL_LOAD_DBZ_P_Z,

    RCL_LOAD_DB2_P_I,
    RCL_LOAD_IR5_P_I,

    RCL_LOAD_DB3_P_D,
    RCL_LOAD_IR5_P_D,

    RCL_LOAD_DB6_P_V,
    RCL_LOAD_AVR_P_V,
    RCL_LOAD_I_P_V,

    RCL_LOAD_DB7_P_N,

    RCL_BITS_COUNT = 62
};


typedef struct {
    uint16_t key;    // IR + timing
    uint64_t value;   // control bits (up to 64)
} Microcode;

typedef struct {
    Microcode *entries;
    size_t count;
} DecodeRom;

extern DecodeRom decodeRom;