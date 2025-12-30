#include "rcl.h"
#include <assert.h>

void rcl_init(RCL *rcl)
{
    for (int i = 0; i < RCL_BITS_COUNT; i++) {
        rcl->ctlSlots[i].value = SIG_0;
    }
    int i = 0;

    // ---- Input Data Latch ----
    rcl->EN_DL_DB      = &rcl->ctlSlots[i++];
    rcl->EN_DL_ADL     = &rcl->ctlSlots[i++];
    rcl->EN_DL_ADH     = &rcl->ctlSlots[i++];

    // ---- Open drain ADH ----
    rcl->EN_0_ADH0     = &rcl->ctlSlots[i++];
    rcl->EN_0_ADH17    = &rcl->ctlSlots[i++];

    // ---- Address buses to register ----
    rcl->LOAD_ADH_ABH  = &rcl->ctlSlots[i++];
    rcl->LOAD_ADL_ABL  = &rcl->ctlSlots[i++];

    // ---- PC ----
    rcl->LOAD_PCL_PCL  = &rcl->ctlSlots[i++];
    rcl->LOAD_ADL_PCL  = &rcl->ctlSlots[i++];
    rcl->EN_I_PC       = &rcl->ctlSlots[i++];
    rcl->EN_PCL_DB     = &rcl->ctlSlots[i++];
    rcl->EN_PCL_ADL    = &rcl->ctlSlots[i++];

    rcl->LOAD_PCH_PCH  = &rcl->ctlSlots[i++];
    rcl->LOAD_ADH_PCH  = &rcl->ctlSlots[i++];
    rcl->EN_PCH_DB     = &rcl->ctlSlots[i++];
    rcl->EN_PCH_ADH    = &rcl->ctlSlots[i++];

    // ---- Pass MOSFET ----
    rcl->EN_SB_ADH     = &rcl->ctlSlots[i++];
    rcl->EN_SB_DB      = &rcl->ctlSlots[i++];

    // ---- Open drain ADL ----
    rcl->EN_0_ADL0     = &rcl->ctlSlots[i++];
    rcl->EN_0_ADL1     = &rcl->ctlSlots[i++];
    rcl->EN_0_ADL2     = &rcl->ctlSlots[i++];

    // ---- Stack Pointer ----
    rcl->EN_S_ADL      = &rcl->ctlSlots[i++];
    rcl->LOAD_SB_S     = &rcl->ctlSlots[i++];
    rcl->HOLD_S_S      = &rcl->ctlSlots[i++];
    rcl->EN_S_SB       = &rcl->ctlSlots[i++];

    // ---- ALU ----
    rcl->LOAD_notDB_ADD = &rcl->ctlSlots[i++];
    rcl->LOAD_DB_ADD    = &rcl->ctlSlots[i++];
    rcl->LOAD_ADL_ADD   = &rcl->ctlSlots[i++];

    rcl->EN_I_AD0C     = &rcl->ctlSlots[i++];
    rcl->EN_DAA        = &rcl->ctlSlots[i++];
    rcl->EN_DSA        = &rcl->ctlSlots[i++];

    rcl->OP_SUMS       = &rcl->ctlSlots[i++];
    rcl->OP_ANDS       = &rcl->ctlSlots[i++];
    rcl->OP_EORS       = &rcl->ctlSlots[i++];
    rcl->OP_ORS        = &rcl->ctlSlots[i++];
    rcl->OP_SRS        = &rcl->ctlSlots[i++];

    rcl->EN_ADD_ADL    = &rcl->ctlSlots[i++];
    rcl->EN_ADD06_SB   = &rcl->ctlSlots[i++];
    rcl->EN_ADD7_SB    = &rcl->ctlSlots[i++];

    rcl->LOAD_0_ADD    = &rcl->ctlSlots[i++];
    rcl->LOAD_SB_ADD   = &rcl->ctlSlots[i++];

    // ---- Accumulator ----
    rcl->LOAD_SB_AC    = &rcl->ctlSlots[i++];
    rcl->EN_AC_DB      = &rcl->ctlSlots[i++];
    rcl->EN_AC_SB      = &rcl->ctlSlots[i++];

    // ---- X ----
    rcl->LOAD_SB_X     = &rcl->ctlSlots[i++];
    rcl->EN_X_SB       = &rcl->ctlSlots[i++];

    // ---- Y ----
    rcl->LOAD_SB_Y     = &rcl->ctlSlots[i++];
    rcl->EN_Y_SB       = &rcl->ctlSlots[i++];

    // ---- Processor Status ----
    rcl->EN_P_DB        = &rcl->ctlSlots[i++];

    // Carry
    rcl->LOAD_DB0_P_C   = &rcl->ctlSlots[i++];
    rcl->LOAD_IR5_P_C   = &rcl->ctlSlots[i++];
    rcl->LOAD_ACR_P_C   = &rcl->ctlSlots[i++];

    // Zero
    rcl->LOAD_DB1_P_Z   = &rcl->ctlSlots[i++];
    rcl->LOAD_DBZ_P_Z   = &rcl->ctlSlots[i++];

    // Interrupt
    rcl->LOAD_DB2_P_I   = &rcl->ctlSlots[i++];
    rcl->LOAD_IR5_P_I   = &rcl->ctlSlots[i++];

    // Decimal
    rcl->LOAD_DB3_P_D   = &rcl->ctlSlots[i++];
    rcl->LOAD_IR5_P_D   = &rcl->ctlSlots[i++];

    // Overflow
    rcl->LOAD_DB6_P_V   = &rcl->ctlSlots[i++];
    rcl->LOAD_AVR_P_V   = &rcl->ctlSlots[i++];
    rcl->LOAD_I_P_V     = &rcl->ctlSlots[i++];

    // Negative
    rcl->LOAD_DB7_P_N   = &rcl->ctlSlots[i++];

    // Safety check
    assert(i == RCL_BITS_COUNT);
}

void rcl_apply(RCL *rcl, uint64_t word)
{
    for (int i = 0; i < RCL_BITS_COUNT; i++) {
        rcl->ctlSlots[i].value =
            (word & (1ULL << i)) ? SIG_1 : SIG_0;
    }
}

uint16_t build_key(Slot *ir, Slot *timing)
{
    uint16_t key = 0;

    // IR bits (8)
    for (int i = 0; i < 8; i++) {
        if (ir[i].value == SIG_1)
            key |= (1u << (8 + i));
    }

    // Timing bits (7), placed above IR
    for (int i = 0; i < 8; i++) {
        if (timing[i].value == SIG_1)
            key |= (1u << i);
    }

    return key;
}

void rcl_eval(
    RCL *rcl,
    Slot *ir,
    Slot *timing
)
{
    // Default: everything off
    for (int i = 0; i < RCL_BITS_COUNT; i++)
        rcl->ctlSlots[i].value = SIG_0;

    uint16_t key = build_key(ir, timing);

    // Lookup
    for (size_t i = 0; i < decodeRom.count; i++) {
        if (decodeRom.entries[i].key == key) {
            rcl_apply(rcl, decodeRom.entries[i].value);
            return;
        }
    }
}

