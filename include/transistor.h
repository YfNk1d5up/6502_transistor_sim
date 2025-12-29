#pragma once

typedef enum {
    SIG_0 = 0,
    SIG_1 = 1,
    SIG_Z = 2   // high impedance
} Signal;

typedef struct {
    Signal value;
} Slot;


typedef struct {
    Slot **slots;
    int n_slots;
    int capacity;
    Slot resolved;
} Node;

extern Slot VDD;
extern Slot GND;

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

int transistor_on(Transistor *t);

void transistor_eval(Transistor *t);

void node_add_slot(Node *node, Slot *s);

void node_resolve(Node *n);

void allocate_node(Node *n, int num_slots, int N);
