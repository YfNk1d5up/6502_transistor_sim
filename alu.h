
#pragma once
#include "gates.h"
#include "fulladder.h"
#include <stdlib.h>

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

void alu_bit_init(ALUBit *bit, Slot *A, Slot *B, Slot *Cin) {
    bit->op_add->value = SIG_0;
    bit->op_and->value = SIG_0;
    bit->op_or->value  = SIG_0;
    bit->op_xor->value = SIG_0;
    bit->op_sub->value = SIG_0;

    // XOR B with op_sub (B ^ sub)
    xor_init(&bit->xor_b_sub, B, bit->op_sub);

    // Full adder uses XORed B
    full_adder_init(&bit->add, A, &bit->xor_b_sub.out.resolved, Cin);
    and_init(&bit->and_gate, A, B);
    or_init(&bit->or_gate, A, B);
    xor_init(&bit->xor_gate, A, B);
}

void alu_bit_eval(ALUBit *bit) {
    // Evaluate B XOR SUB
    xor_eval(&bit->xor_b_sub);

    // Evaluate basic operations
    full_adder_eval(&bit->add);
    and_eval(&bit->and_gate);
    or_eval(&bit->or_gate);
    xor_eval(&bit->xor_gate);

    // Selection logic
    and_init(&bit->sel_add, &bit->add.sum, bit->op_add);
    and_init(&bit->sel_and, &bit->and_gate.out.resolved, bit->op_and);
    and_init(&bit->sel_or,  &bit->or_gate.out.resolved,  bit->op_or);
    and_init(&bit->sel_xor, &bit->xor_gate.out.resolved, bit->op_xor);

    and_eval(&bit->sel_add);
    and_eval(&bit->sel_and);
    and_eval(&bit->sel_or);
    and_eval(&bit->sel_xor);

    // Combine selected outputs
    or_init(&bit->or1, &bit->sel_add.out.resolved, &bit->sel_and.out.resolved);
    or_init(&bit->or2, &bit->sel_or.out.resolved,  &bit->sel_xor.out.resolved);
    or_init(&bit->or3, &bit->or1.out.resolved,     &bit->or2.out.resolved);

    or_eval(&bit->or1);
    or_eval(&bit->or2);
    or_eval(&bit->or3);

    // Final output
    bit->result = bit->or3.out.resolved;
}

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

void alu_nbit_init(ALUNBit *alu, int N, Slot *A, Slot *B) {
    alu->N = N;
    alu->bits = malloc(sizeof(ALUBit) * N);
    alu->A = A;
    alu->B = B;
    alu->result = malloc(sizeof(Slot*) * N);

    // Initialize control pins
    alu->op_add.value = SIG_0;
    alu->op_and.value = SIG_0;
    alu->op_or.value  = SIG_0;
    alu->op_xor.value = SIG_0;
    alu->op_sub.value = SIG_0;

    for (int i = 0; i < N; i++) {
        ALUBit *b = &alu->bits[i];
        b->op_add = &alu->op_add;
        b->op_and = &alu->op_and;
        b->op_or  = &alu->op_or;
        b->op_xor = &alu->op_xor;
        b->op_sub = &alu->op_sub;

        Slot *cin = i == 0 ? &alu->op_sub : &alu->bits[i-1].add.cout;
        alu_bit_init(b, &A[i], &B[i], cin);
    }

    // --- Setup permanent flag hardware ---
    int Nbits = alu->N;
    alu->flags.or_chain_len = Nbits - 1;

    // OR chain for zero flag
    or_init(&alu->flags.or_chain[0], &alu->bits[0].result, &alu->bits[1].result);
    for (int i = 2; i < Nbits; i++) {
        or_init(&alu->flags.or_chain[i-1],
                &alu->flags.or_chain[i-2].out.resolved,
                &alu->bits[i].result);
    }

    // NOT gate for zero
    not_init(&alu->flags.not_z, &alu->flags.or_chain[Nbits-2].out.resolved);

    // XOR for overflow (MSB carry-in vs carry-out)
    xor_init(&alu->flags.xor_v,
             &alu->bits[Nbits-1].add.cout,
             &alu->bits[Nbits-2].add.cout);
}

void alu_nbit_eval(ALUNBit *alu) {
    // Evaluate ALU bits
    for (int i = 0; i < alu->N; i++) {
        alu_bit_eval(&alu->bits[i]);
        alu->result[i] = &alu->bits[i].result;
    }

    // --- Evaluate flags using permanent gates ---
    for (int i = 0; i < alu->flags.or_chain_len; i++)
        or_eval(&alu->flags.or_chain[i]);

    not_eval(&alu->flags.not_z);
    xor_eval(&alu->flags.xor_v);

    // Assign flags
    alu->flags.N = alu->bits[alu->N-1].result;               // Negative
    alu->flags.C = alu->bits[alu->N-1].add.cout;            // Carry
    alu->flags.Z = alu->flags.not_z.out.resolved;           // Zero
    alu->flags.V = alu->flags.xor_v.out.resolved;           // Overflow
}
