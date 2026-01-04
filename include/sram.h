#pragma once
#include "register.h"
#include "transistor.h"
#include <stdint.h>

typedef struct {
    int N;
    Slot *CLK;

    Slot *dummy;
    Slot **one;
    Slot **zero;

    int size;
    Slot *RnotW;

    Slot **dataBusD;
    Slot *dataBusQ;
    Slot **addressBusD;

    Slot **loadreg; // Load signal for registers
    Slot **enreg;
    NBitRegister **regRAMs;

} SRAM;

uint32_t slot_bus_to_int(Slot **bus, int size);

void sram_init(
    SRAM *ram, 
    int N,
    Slot *CLK,
    Slot *dummy,
    Slot **one,
    Slot **zero,
    int size,
    Slot *RnotW,
    Slot **dataBusD,
    Slot **addressBusD
);

void sram_eval(SRAM *ram);
