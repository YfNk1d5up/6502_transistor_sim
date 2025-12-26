#ifndef HELPERS_H
#define HELPERS_H

#include "transistor.h"
#include "register.h"

const char* S(Signal s) {
    switch (s) {
        case SIG_0: return "0";
        case SIG_1: return "1";
        case SIG_Z: return "Z";
    }
    return "?";
}

static void print_bus(const char *name, Node *bus, int N) {
    printf("%s: ", name);
    for (int i = 0; i < N; i++)
        printf("%s ", S(bus[i].resolved.value));
    printf("\n");
}

void print_slot_ptr(const char *name, Slot *slot) {
    printf("%s = ", name);
    printf("%s", S(slot->value));
    printf("\n");
}

static void print_slots(const char *name, Slot *s, int N) {
    printf("%s: ", name);
    for (int i = 0; i < N; i++)
        printf("%s ", S(s[i].value));
    printf("\n");
}

static void print_slots_ptr(const char *name, Slot **s, int N) {
    printf("%s: ", name);
    for (int i = 0; i < N; i++)
        printf("%s ", S(s[i]->value));
    printf("\n");
}

static void print_bus_slots(const char *name, Node *bus, int N) {
    printf("%s (all slots):\n", name);

    for (int bit = 0; bit < N; bit++) {
        printf("  [%d] ", bit);

        for (int s = 0; s < bus[bit].n_slots; s++) {
            printf("%s ", S(bus[bit].slots[s]->value));
        }

        printf("-> %s\n", S(bus[bit].resolved.value));
    }
}


#endif
