#include "decoderom.h"

#define RCL_BIT(x) (1ULL << (x))

static Microcode test_microcode[] = {
    {
        .key = (0x00 << 8) | (0x01), // Load PC = 0000 on ABL/ABH, Increment PC on ADL and ADH
        .value =
            RCL_BIT(RCL_RnotW) |
            RCL_BIT(RCL_LOAD_PCL_PCL) |
            RCL_BIT(RCL_EN_PCL_ADL) |
            RCL_BIT(RCL_LOAD_PCH_PCH) |
            RCL_BIT(RCL_EN_PCH_ADH) |
            RCL_BIT(RCL_EN_I_PC) |
            RCL_BIT(RCL_LOAD_ADL_ABL) |
            RCL_BIT(RCL_LOAD_ADH_ABH) 
    },
    {
        .key = (0x00 << 8) | (0x02), // Read RAM at 0000 
        .value =
            RCL_BIT(RCL_RnotW) |
            RCL_BIT(RCL_EN_DL_DB) |
            RCL_BIT(RCL_LOAD_PCL_PCL) |
            RCL_BIT(RCL_EN_PCL_ADL) |
            RCL_BIT(RCL_LOAD_PCH_PCH) |
            RCL_BIT(RCL_EN_PCH_ADH) |
            RCL_BIT(RCL_LOAD_ADL_ABL) |
            RCL_BIT(RCL_LOAD_ADH_ABH) 
    },
    {
        .key = (0x01 << 8) | (0x04), // Load PC = 0001 on ABL/ABH, Increment PC on ADL and ADH
        .value =
            RCL_BIT(RCL_RnotW) |
            RCL_BIT(RCL_LOAD_PCL_PCL) |
            RCL_BIT(RCL_EN_PCL_ADL) |
            RCL_BIT(RCL_LOAD_PCH_PCH) |
            RCL_BIT(RCL_EN_PCH_ADH) |
            RCL_BIT(RCL_EN_I_PC) |
            RCL_BIT(RCL_LOAD_ADL_ABL) |
            RCL_BIT(RCL_LOAD_ADH_ABH) 
    },
    {
        .key = (0x01 << 8) | (0x08), // Read RAM at 0001
        .value =
            RCL_BIT(RCL_RnotW) |
            RCL_BIT(RCL_EN_DL_DB)
    },
};

static Microcode adding_loop_microcode[] = {
    {
        // 00000000
        .key = (0x00 << 8) | (0x00), // Keep PC loop without incr
        .value =
            RCL_BIT(RCL_RnotW) |
            RCL_BIT(RCL_LOAD_PCL_PCL) |
            RCL_BIT(RCL_EN_PCL_ADL) |
            RCL_BIT(RCL_LOAD_PCH_PCH) |
            RCL_BIT(RCL_EN_PCH_ADH) |
            RCL_BIT(RCL_LOAD_ADL_ABL) |
            RCL_BIT(RCL_LOAD_ADH_ABH) 
    },
    {
        // 00000001
        .key = (0x00 << 8) | (0x01), // Load PC = 0000 on ABL/ABH and incr
        .value =
            RCL_BIT(RCL_RnotW) |
            RCL_BIT(RCL_LOAD_PCL_PCL) |
            RCL_BIT(RCL_EN_PCL_ADL) |
            RCL_BIT(RCL_LOAD_PCH_PCH) |
            RCL_BIT(RCL_EN_PCH_ADH) |
            RCL_BIT(RCL_EN_I_PC) |
            RCL_BIT(RCL_LOAD_ADL_ABL) |
            RCL_BIT(RCL_LOAD_ADH_ABH) 
    },
    {
        // 00000000
        .key = (0x02 << 8) | (0x00), // nothing
        .value =
            RCL_BIT(RCL_RnotW) |
            RCL_BIT(RCL_LOAD_PCL_PCL) |
            RCL_BIT(RCL_EN_PCL_ADL) |
            RCL_BIT(RCL_LOAD_PCH_PCH) |
            RCL_BIT(RCL_EN_PCH_ADH) |
            RCL_BIT(RCL_LOAD_ADL_ABL) |
            RCL_BIT(RCL_LOAD_ADH_ABH) 
    },
    {
        // 00000001
        .key = (0x02 << 8) | (0x01), // Load PC = 0000 on ABL/ABH, incr PC
        .value =
            RCL_BIT(RCL_RnotW) |
            RCL_BIT(RCL_LOAD_PCL_PCL) |
            RCL_BIT(RCL_EN_PCL_ADL) |
            RCL_BIT(RCL_LOAD_PCH_PCH) |
            RCL_BIT(RCL_EN_PCH_ADH) |
            RCL_BIT(RCL_EN_I_PC) |
            RCL_BIT(RCL_LOAD_ADL_ABL) |
            RCL_BIT(RCL_LOAD_ADH_ABH) 
    },
    {
        // 00100010
        .key = (0x02 << 8) | (0x02), // Read RAM at 0001 into DB and 
                                     // LOAD DB into B ALU and 0 into A ALU
                                     // sum OP
        .value =
            RCL_BIT(RCL_RnotW) |
            RCL_BIT(RCL_EN_DL_DB) |
            RCL_BIT(RCL_LOAD_DB_ADD) |
            RCL_BIT(RCL_LOAD_0_ADD) |
            RCL_BIT(RCL_OP_SUMS) |
            RCL_BIT(RCL_LOAD_PCL_PCL) |
            RCL_BIT(RCL_EN_PCL_ADL) |
            RCL_BIT(RCL_LOAD_PCH_PCH) |
            RCL_BIT(RCL_EN_PCH_ADH) |
            RCL_BIT(RCL_LOAD_ADL_ABL) |
            RCL_BIT(RCL_LOAD_ADH_ABH) 
    },

    {
        // 
        .key = (0x02 << 8) | (0x04), //  Result on SB and load into ALU A and sum again
        .value =
            RCL_BIT(RCL_RnotW) |
            
            RCL_BIT(RCL_EN_ADD06_SB) |
            RCL_BIT(RCL_EN_ADD7_SB) |
            
            RCL_BIT(RCL_OP_SUMS) |
            RCL_BIT(RCL_LOAD_PCL_PCL) |
            RCL_BIT(RCL_EN_PCL_ADL) |
            RCL_BIT(RCL_LOAD_PCH_PCH) |
            RCL_BIT(RCL_EN_PCH_ADH) |
            RCL_BIT(RCL_LOAD_ADL_ABL) |
            RCL_BIT(RCL_LOAD_ADH_ABH) 
    },
    {
        .key = (0x02 << 8) | (0x08), // Result output on SB & sum op
        .value =
            RCL_BIT(RCL_RnotW) |
            RCL_BIT(RCL_LOAD_SB_ADD) |
            RCL_BIT(RCL_EN_ADD06_SB) |
            RCL_BIT(RCL_EN_ADD7_SB) |
            RCL_BIT(RCL_OP_SUMS) |
            RCL_BIT(RCL_LOAD_PCL_PCL) |
            RCL_BIT(RCL_EN_PCL_ADL) |
            RCL_BIT(RCL_LOAD_PCH_PCH) |
            RCL_BIT(RCL_EN_PCH_ADH) |
            RCL_BIT(RCL_LOAD_ADL_ABL) |
            RCL_BIT(RCL_LOAD_ADH_ABH) 
    },
    {
        .key = (0x02 << 8) | (0x10), // Nothing
        .value =
            RCL_BIT(RCL_RnotW) |
            RCL_BIT(RCL_LOAD_PCL_PCL) |
            RCL_BIT(RCL_EN_PCL_ADL) |
            RCL_BIT(RCL_LOAD_PCH_PCH) |
            RCL_BIT(RCL_EN_PCH_ADH) |
            RCL_BIT(RCL_LOAD_ADL_ABL) |
            RCL_BIT(RCL_LOAD_ADH_ABH) 
    },
    {
        .key = (0x02 << 8) | (0x20), // Nothing
        .value =
            RCL_BIT(RCL_RnotW) |
            RCL_BIT(RCL_LOAD_PCL_PCL) |
            RCL_BIT(RCL_EN_PCL_ADL) |
            RCL_BIT(RCL_LOAD_PCH_PCH) |
            RCL_BIT(RCL_EN_PCH_ADH) |
            RCL_BIT(RCL_LOAD_ADL_ABL) |
            RCL_BIT(RCL_LOAD_ADH_ABH) 
    },
    {
        .key = (0x02 << 8) | (0x40), // Nothing
        .value =
            RCL_BIT(RCL_RnotW) |
            RCL_BIT(RCL_LOAD_PCL_PCL) |
            RCL_BIT(RCL_EN_PCL_ADL) |
            RCL_BIT(RCL_LOAD_PCH_PCH) |
            RCL_BIT(RCL_EN_PCH_ADH) |
            RCL_BIT(RCL_LOAD_ADL_ABL) |
            RCL_BIT(RCL_LOAD_ADH_ABH) 
    },

};

DecodeRom decodeRom = {
    .entries = adding_loop_microcode,
    .count = sizeof(adding_loop_microcode) / sizeof(adding_loop_microcode[0])
};
