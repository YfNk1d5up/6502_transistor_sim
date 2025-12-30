#include "decoderom.h"

#define RCL_BIT(x) (1ULL << (x))

static Microcode microcode[] = {
    {
        .key = (0xF0 << 8) | (0x80), // test opcode on T0
        .value =
            RCL_BIT(RCL_LOAD_PCL_PCL) |
            RCL_BIT(RCL_EN_PCL_ADL) |
            RCL_BIT(RCL_LOAD_PCH_PCH) |
            RCL_BIT(RCL_EN_PCH_ADH) |
            RCL_BIT(RCL_EN_I_PC) 
    },
    {
        .key = (0xF0 << 8) | (0x40), // test opcode on T1X
        .value =
            RCL_BIT(RCL_LOAD_PCL_PCL) |
            RCL_BIT(RCL_EN_PCL_ADL) |
            RCL_BIT(RCL_LOAD_PCH_PCH) |
            //RCL_BIT(RCL_EN_PCH_ADH) |
            RCL_BIT(RCL_EN_I_PC) 
    },
};

DecodeRom decodeRom = {
    .entries = microcode,
    .count = sizeof(microcode) / sizeof(microcode[0])
};
