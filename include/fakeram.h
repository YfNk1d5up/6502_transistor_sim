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

    uint32_t *mem;

} FAKERAM;

uint32_t ram_mask(int N);

uint32_t slot_bus_to_int(Slot **bus, int size);

void fakeram_init(
    FAKERAM *ram, 
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

void fakeram_eval(FAKERAM *ram);

int fakeram_load_hex_words(
    FAKERAM *ram,
    const char *filename,
    uint32_t start_addr
);