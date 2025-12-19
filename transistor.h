#ifndef TRANSISTOR_H
#define TRANSISTOR_H

typedef enum {
    SIG_0 = 0,
    SIG_1 = 1,
    SIG_Z = 2   // high impedance
} Signal;

typedef struct {
    Signal value;
} Slot;


typedef struct {
    Slot *slots;
    int n_slots;
    Slot resolved;
} Node;

Slot VDD = { SIG_1 };
Slot GND = { SIG_0 };

typedef enum {
    NMOS,
    PMOS
} TransistorType;

typedef struct {
    TransistorType type;
    Slot *gate;
    Slot *source;
    Slot *drain;
} Transistor;

int transistor_on(Transistor *t) {
    if (t->type == NMOS)
        return t->gate->value == SIG_1;
    else // PMOS
        return t->gate->value == SIG_0;
}

void transistor_eval(Transistor *t) {
    if (transistor_on(t)) {
        t->drain->value = t->source->value;
    }
}

void node_resolve(Node *n) {
    int has0 = 0;
    int has1 = 0;
    for (int i = 0; i < n->n_slots; i++) {
        if (n->slots[i].value == SIG_0) has0 = 1;
        if (n->slots[i].value == SIG_1) has1 = 1;
    }
    if (has0 && has1) {
        n->resolved.value = SIG_Z; // contention
    } else if (has1) {
        n->resolved.value = SIG_1;
    } else if (has0) {
        n->resolved.value = SIG_0;
    } else {
        n->resolved.value = SIG_Z; // floating
    }
}

#endif // TRANSISTOR_H
