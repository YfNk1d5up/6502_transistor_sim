#include "sram.h"
#include <stdlib.h>

uint32_t slot_bus_to_int(Slot **bus, int size)
{
    uint32_t value = 0;
    for (int bit = 0; bit < size; bit++) {
        if (bus[bit]->value)
            value |= (1u << bit);
    }
    return value;
}

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
) {
    ram->N = N;
    ram->CLK = CLK;
    ram->dummy = dummy;
    ram->one = one;
    ram->zero= zero;
    ram->size = size;
    ram->RnotW = RnotW;
    ram->dataBusD = dataBusD;
    ram->addressBusD = addressBusD;
 
    ram->dataBusQ = malloc(sizeof(Slot) * ram->N);
    for (int i=0; i < ram->N; i++)
        ram->dataBusQ[i].value = SIG_Z;

    ram->loadreg = malloc(sizeof(Slot*) * (1u << ram->size));
    ram->enreg = malloc(sizeof(Slot*) * (1u << ram->size));
    ram->regRAMs = malloc(sizeof(NBitRegister*) * (1u << ram->size));

    for (int i = 0; i < 1u << ram->size; i++) {
        ram->loadreg[i] = malloc(sizeof(Slot));
        ram->loadreg[i]->value = SIG_0;

        ram->enreg[i] = malloc(sizeof(Slot));
        ram->enreg[i]->value = SIG_0;

        NBitRegister *reg = malloc(sizeof(NBitRegister));
        nreg_init(reg, ram->N, 1, 1, ram->CLK);
        nreg_add_load_port(reg, 0, ram->dataBusD, ram->loadreg[i]);
        nreg_add_enable_port(reg, 0, ram->dataBusQ, ram->dummy, ram->enreg[i]);
        ram->regRAMs[i] = reg;
    }
};

void sram_eval(SRAM *ram)
{
    uint32_t addr = slot_bus_to_int(
        ram->addressBusD,
        ram->size
    );
    for (int i = 0; i < 1u << ram->size; i++) {
        ram->loadreg[i]->value = (addr == (uint32_t)i)
            ? SIG_1
            : SIG_0;
        ram->enreg[i]->value = (ram->RnotW == SIG_1) ? SIG_0 : ram->loadreg[i]->value;
    }
    nreg_eval(ram);
}
