#include "decoderom.h"

#define RCL_BIT(x) (1ULL << (x))

static Microcode microcode[] = {
    {
        .key = (0x00 << 8) | (0x02), // Load PC = 0000 on ABL/ABH, Increment PC on ADL and ADH
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
        .key = (0x00 << 8) | (0x04), // Read RAM at 0000 
        .value =
            RCL_BIT(RCL_RnotW) |
            RCL_BIT(RCL_EN_DL_DB) |
            RCL_BIT(RCL_LOAD_PCL_PCL) |
            RCL_BIT(RCL_EN_PCL_ADL) |
            RCL_BIT(RCL_LOAD_PCH_PCH) |
            RCL_BIT(RCL_EN_PCH_ADH) |
            RCL_BIT(RCL_EN_I_PC) |
            RCL_BIT(RCL_LOAD_ADL_ABL) |
            RCL_BIT(RCL_LOAD_ADH_ABH) 
    },
    {
        .key = (0x00 << 8) | (0x08), // Load PC = 0001 on ABL/ABH, Increment PC on ADL and ADH
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
        .key = (0x00 << 8) | (0x10), // Read RAM at 0001
        .value =
            RCL_BIT(RCL_RnotW) |
            RCL_BIT(RCL_EN_DL_DB)
    },
};

DecodeRom decodeRom = {
    .entries = microcode,
    .count = sizeof(microcode) / sizeof(microcode[0])
};
