#pragma once
#include "gates.h"

// -------------------
// 1-bit Full Adder
// -------------------

typedef struct {
    XORGate xor1;   // A XOR B
    XORGate xor2;   // (A XOR B) XOR Cin
    ANDGate and1;   // A AND B
    ANDGate and2;   // Cin AND (A XOR B)
    ORGate  or1;    // Cout = and1 OR and2

    Slot sum;       // final sum
    Slot cout;      // final carry-out
} FullAdder1Bit;

// Initialize 1-bit full adder
void full_adder_init(FullAdder1Bit *fa, Slot *A, Slot *B, Slot *Cin);

// Evaluate 1-bit full adder
void full_adder_eval(FullAdder1Bit *fa);

// -------------------
// N-bit Full Adder
// -------------------

typedef struct {
    int N;
    FullAdder1Bit *bits;
    Slot **sum;  // array of N sum slots
    Slot *cout;  // final carry-out
} FullAdderNBit;

// Initialize N-bit full adder
void full_adder_nbit_init(FullAdderNBit *faN, Slot *A, Slot *B, int N);

// Evaluate N-bit full adder
void full_adder_nbit_eval(FullAdderNBit *faN);
