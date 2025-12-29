#pragma once
#include "transistor.h"

// -----NOT-----

typedef struct {
    Transistor pmos;
    Transistor nmos;
    Slot *out_slots[2];
    Node out;
} NOTGate;

void not_init(NOTGate *g, Slot *in);

void not_eval(NOTGate *g);

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

void nand_init(NANDGate *g, Slot *a, Slot *b);

void nand_eval(NANDGate *g);


// -----AND-----

typedef struct {
    NANDGate nand;
    NOTGate not;
    Node out;
} ANDGate;

void and_init(ANDGate *g, Slot *a, Slot *b);

void and_eval(ANDGate *g);

// -----OR-----

typedef struct {
    NOTGate na, nb;
    NANDGate nand;
    Node out;
} ORGate;

void or_init(ORGate *g, Slot *a, Slot *b);

void or_eval(ORGate *g);

// ---------- XOR ----------

typedef struct {
    NANDGate n1, n2, n3, n4;
    Node out;
} XORGate;

void xor_init(XORGate *g, Slot *a, Slot *b);

void xor_eval(XORGate *g);

// ---------- NOR ----------

typedef struct {
    ORGate or_gate;
    NOTGate not_gate;
    Node out;
} NORGate;

void nor_init(NORGate *g, Slot *a, Slot *b);

void nor_eval(NORGate *g);

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

void tristate_init(TriStateGate *g, Slot *in, Slot *en);

void tristate_eval(TriStateGate *g);
