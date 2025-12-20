#ifndef GATES_H
#define GATES_H
#include "transistor.h"

// -----NOT-----

typedef struct {
    Transistor pmos;
    Transistor nmos;
    Slot *out_slots[2];
    Node out;
} NOTGate;

void not_init(NOTGate *g, Slot *in) {
    g->out_slots[0] = malloc(sizeof(Slot));
    g->out_slots[1] = malloc(sizeof(Slot));
    g->out.slots = g->out_slots;
    g->out.n_slots = 2;

    g->pmos = (Transistor){ PMOS, in, &VDD, g->out_slots[0] };
    g->nmos = (Transistor){ NMOS, in, &GND, g->out_slots[1] };
}

void not_eval(NOTGate *g) {
    g->out_slots[0]->value = SIG_Z;
    g->out_slots[1]->value = SIG_Z;

    transistor_eval(&g->pmos);
    transistor_eval(&g->nmos);

    node_resolve(&g->out);
}

// -----NAND-----

typedef struct {
    // PMOS
    Transistor p1, p2;

    // NMOS
    Transistor n1, n2;

    // Intermediate NMOS node
    Slot *inter_slot[1];
    Node inter;

    // Output node
    Slot *out_slots[3];
    Node out;
} NANDGate;

void nand_init(NANDGate *g, Slot *a, Slot *b) {
    // Intermediate
    g->inter_slot[0] = malloc(sizeof(Slot));
    g->inter.slots = g->inter_slot;
    g->inter.n_slots = 1;

    // Output
    g->out_slots[0] = malloc(sizeof(Slot));
    g->out_slots[1] = malloc(sizeof(Slot));
    g->out_slots[2] = malloc(sizeof(Slot));
    g->out.slots = g->out_slots;
    g->out.n_slots = 3;

    // PMOS in parallel
    g->p1 = (Transistor){ PMOS, a, &VDD, g->out_slots[0] };
    g->p2 = (Transistor){ PMOS, b, &VDD, g->out_slots[1] };

    // NMOS in series
    g->n1 = (Transistor){ NMOS, a, &GND, g->inter_slot[0] };
    g->n2 = (Transistor){ NMOS, b, &g->inter.resolved, g->out_slots[2] };
}

void nand_eval(NANDGate *g) {
    g->inter_slot[0]->value = SIG_Z;
    for (int i = 0; i < 3; i++) g->out_slots[i]->value = SIG_Z;

    transistor_eval(&g->p1);
    transistor_eval(&g->p2);
    transistor_eval(&g->n1);

    node_resolve(&g->inter);
    transistor_eval(&g->n2);

    node_resolve(&g->out);
}


// -----AND-----

typedef struct {
    NANDGate nand;
    NOTGate not;
    Node out;
} ANDGate;

void and_init(ANDGate *g, Slot *a, Slot *b) {
    nand_init(&g->nand, a, b);
    not_init(&g->not, &g->nand.out.resolved);
}

void and_eval(ANDGate *g) {
    nand_eval(&g->nand);
    not_eval(&g->not);
    g->out = g->not.out;

}

// -----OR-----

typedef struct {
    NOTGate na, nb;
    NANDGate nand;
    Node out;
} ORGate;

static inline void or_init(ORGate *g, Slot *a, Slot *b) {
    not_init(&g->na, a);
    not_init(&g->nb, b);
    nand_init(&g->nand, &g->na.out.resolved, &g->nb.out.resolved);
}

static inline void or_eval(ORGate *g) {
    not_eval(&g->na);
    not_eval(&g->nb);
    nand_eval(&g->nand);
    g->out = g->nand.out;
}

// ---------- XOR ----------

typedef struct {
    NANDGate n1, n2, n3, n4;
    Node out;
} XORGate;

static inline void xor_init(XORGate *g, Slot *a, Slot *b) {
    nand_init(&g->n1, a, b);
    nand_init(&g->n2, a, &g->n1.out.resolved);
    nand_init(&g->n3, b, &g->n1.out.resolved);
    nand_init(&g->n4, &g->n2.out.resolved, &g->n3.out.resolved);
}

static inline void xor_eval(XORGate *g) {
    nand_eval(&g->n1);
    nand_eval(&g->n2);
    nand_eval(&g->n3);
    nand_eval(&g->n4);
    g->out = g->n4.out;
}

// ---------- NOR ----------

// -----NOR-----

typedef struct {
    ORGate or_gate;
    NOTGate not_gate;
    Node out;
} NORGate;

static inline void nor_init(NORGate *g, Slot *a, Slot *b) {
    or_init(&g->or_gate, a, b);
    not_init(&g->not_gate, &g->or_gate.out.resolved);
}

static inline void nor_eval(NORGate *g) {
    or_eval(&g->or_gate);
    not_eval(&g->not_gate);
    g->out = g->not_gate.out;
}

typedef struct {
    // Enable logic
    NOTGate not_in;
    NANDGate and_p;
    ANDGate and_n;

    // Output node
    Slot *out_slots[2];
    Node out;

    // Transistors
    Transistor pmos;
    Transistor nmos;
} TriStateGate;

void tristate_init(TriStateGate *g, Slot *in, Slot *en) {
    g->out_slots[0] = malloc(sizeof(Slot));
    g->out_slots[1] = malloc(sizeof(Slot));

    g->out.slots = g->out_slots;
    g->out.n_slots = 2;

    not_init(&g->not_in, in);
    nand_init(&g->and_p, in, en);
    and_init(&g->and_n, &g->not_in.out.resolved, en);

    g->pmos = (Transistor){
        PMOS,
        &g->and_p.out.resolved,
        &VDD,
        g->out_slots[0]
    };
    g->nmos = (Transistor){
        NMOS,
        &g->and_n.out.resolved,
        &GND,
        g->out_slots[1]
    };
}

void tristate_eval(TriStateGate *g) {
    g->out_slots[0]->value = SIG_Z;
    g->out_slots[1]->value = SIG_Z;

    not_eval(&g->not_in);
    nand_eval(&g->and_p);
    and_eval(&g->and_n);

    transistor_eval(&g->pmos);
    transistor_eval(&g->nmos);

    node_resolve(&g->out);
}

#endif // GATES_H
