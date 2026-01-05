#include "fakeram.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

uint32_t ram_mask(int N)
{
    return (N >= 32) ? 0xFFFFFFFFu : ((1u << N) - 1);
}

uint32_t slot_bus_to_int(Slot **bus, int size)
{
    uint32_t value = 0;
    for (int bit = 0; bit < size; bit++) {
        if (bus[bit]->value)
            value |= (1u << bit);
    }
    return value;
}

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

    /* Allocate memory array */
    ram->mem = calloc(1u << ram->size, sizeof(uint32_t));
};

void fakeram_eval(FAKERAM *ram)
{
    uint32_t addr = slot_bus_to_int(ram->addressBusD, ram->size);
    addr &= ((1u << ram->size) - 1);

    uint32_t mask = ram_mask(ram->N);

    /* WRITE */
    if (ram->RnotW->value == SIG_0) {
        uint32_t value = slot_bus_to_int(ram->dataBusD, ram->N) & mask;
        ram->mem[addr] = value;

        for (uint32_t i = 0; i < ram->N; i++)
            ram->dataBusQ[i].value = SIG_Z;
        return;
    }

    /* READ */
    uint32_t value = ram->mem[addr] & mask;
    for (uint32_t i = 0; i < ram->N; i++) {
        ram->dataBusQ[i].value =
            (value & (1u << i)) ? SIG_1 : SIG_0;
    }
}


int fakeram_load_hex_words(
    FAKERAM *ram,
    const char *filename,
    uint32_t start_addr
) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fakeram_load_hex_words");
        return -1;
    }

    uint32_t addr = start_addr;
    uint32_t mask = ram_mask(ram->N);

    /* expected hex digits per line */
    int hex_digits = (ram->N + 3) / 4;

    char line[128];
    while (fgets(line, sizeof(line), f)) {
        /* skip empty lines / comments */
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0' || *p == '#')
            continue;

        /* parse hex value */
        unsigned long value;
        if (sscanf(p, "%lx", &value) != 1) {
            fprintf(stderr, "Invalid hex line: %s", line);
            fclose(f);
            return -1;
        }

        /* Optional: strict width check */
        int digits = 0;
        for (char *q = p; isxdigit((unsigned char)*q); q++)
            digits++;

        if (digits > hex_digits) {
            fprintf(stderr,
                "Hex width too large (%d digits, expected %d)\n",
                digits, hex_digits);
            fclose(f);
            return -1;
        }

        if (addr >= (1u << ram->size)) {
            fprintf(stderr, "RAM overflow at address %u\n", addr);
            fclose(f);
            return -1;
        }

        ram->mem[addr++] = ((uint32_t)value) & mask;
    }

    fclose(f);
    return 0;
}
