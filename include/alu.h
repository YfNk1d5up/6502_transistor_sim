
#pragma once
#include "gates.h"
#include "fulladder.h"

// --- 1-bit ALU ---

typedef struct {
    // Basic operations
    FullAdder1Bit add;
    ANDGate and_gate;
    ORGate  or_gate;
    XORGate xor_gate;

    // Selection AND gates
    ANDGate sel_add;
    ANDGate sel_and;
    ANDGate sel_or;
    ANDGate sel_xor;

    // OR gates to combine selection outputs
    ORGate or1;
    ORGate or2;
    ORGate or3;

    // Slots for outputs
    Slot sum;
    Slot and_out;
    Slot or_out;
    Slot xor_out;
    Slot result;

    // Subtraction control
    Slot *op_add;
    Slot *op_and;
    Slot *op_or;
    Slot *op_xor;
    Slot *op_sub;  // if 1, B is inverted for subtraction

    // Internal XOR for B in subtraction
    XORGate xor_b_sub;
} ALUBit;

void alu_bit_init(ALUBit *bit, Slot *A, Slot *B, Slot *Cin);

void alu_bit_eval(ALUBit *bit);

// --- N-bit ALU ---

typedef struct {
    Slot C;  // Carry
    Slot Z;  // Zero
    Slot N;  // Negative (MSB)
    Slot V;  // Overflow

    // Hardware gates for flags
    NOTGate not_z;
    ORGate  or_chain[64];   // Max 2*N, adjust as needed
    int     or_chain_len;
    XORGate xor_v;
} ALUFlags;

typedef struct {
    int N;
    ALUBit *bits;        // Array of ALUBits
    Slot *A;
    Slot *B;
    Slot **result;       // Output bus
    ALUFlags flags;

    // Shared control pins
    Slot op_add;
    Slot op_and;
    Slot op_or;
    Slot op_xor;
    Slot op_sub;
} ALUNBit;

void alu_nbit_init(ALUNBit *alu, int N, Slot *A, Slot *B);

void alu_nbit_eval(ALUNBit *alu);
