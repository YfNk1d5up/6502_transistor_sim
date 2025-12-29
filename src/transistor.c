#include <stdlib.h>
#include "transistor.h"

Slot VDD = { SIG_1 };
Slot GND = { SIG_0 };

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

void node_add_slot(Node *node, Slot *s) {
    if (node->n_slots >= node->capacity) {
        node->capacity = node->capacity ? node->capacity * 2 : 8;
        node->slots = realloc(node->slots, sizeof(Slot*) * node->capacity);
    }
    node->slots[node->n_slots++] = s;
}

void node_resolve(Node *n) {
    int has0 = 0;
    int has1 = 0;
    for (int i = 0; i < n->n_slots; i++) {
        if (n->slots[i]->value == SIG_0) has0 = 1;
        if (n->slots[i]->value == SIG_1) has1 = 1;
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

void allocate_node(Node *n, int num_slots, int N) {
    n = malloc(sizeof(Node) * N);
    for (int i = 0; i < N; i++) {
        n[i].slots = malloc(sizeof(Slot*) * num_slots);
        n[i].n_slots = num_slots;
        n[i].capacity = num_slots;
        for (int j = 0; j < num_slots; j++) {
            n[i].slots[j] = malloc(sizeof(Slot));
            n[i].slots[j]->value = SIG_Z;
        }
        n[i].resolved.value = SIG_Z;
    }
}
