#pragma once
#include "gates.h"
#include "fulladder.h"
#include <stdlib.h>

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

    // Control pins
    Slot *op_add;
    Slot *op_and;
    Slot *op_or;
    Slot *op_xor;
} ALUBit;

// Initialize one 1-bit ALU
void alu_bit_init(ALUBit *bit, Slot *A, Slot *B, Slot *Cin) {

    bit->op_add->value = SIG_0;
    bit->op_and->value = SIG_0;
    bit->op_or->value  = SIG_0;
    bit->op_xor->value = SIG_0;

    // Initialize basic gates
    full_adder_init(&bit->add, A, B, Cin);
    and_init(&bit->and_gate, A, B);
    or_init(&bit->or_gate, A, B);
    xor_init(&bit->xor_gate, A, B);
}

// Evaluate one 1-bit ALU
void alu_bit_eval(ALUBit *bit) {
    // Evaluate basic operations
    full_adder_eval(&bit->add);
    and_eval(&bit->and_gate);
    or_eval(&bit->or_gate);
    xor_eval(&bit->xor_gate);

    // Selection gates
    and_init(&bit->sel_add, &bit->add.sum, bit->op_add);
    and_init(&bit->sel_and, &bit->and_gate.out.resolved, bit->op_and);
    and_init(&bit->sel_or,  &bit->or_gate.out.resolved,  bit->op_or);
    and_init(&bit->sel_xor, &bit->xor_gate.out.resolved, bit->op_xor);

    and_eval(&bit->sel_add);
    and_eval(&bit->sel_and);
    and_eval(&bit->sel_or);
    and_eval(&bit->sel_xor);

    // Combine selection outputs
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
    int N;
    ALUBit *bits;  // Array of ALUBit
    Slot *A;
    Slot *B;
    Slot **result;  // Array of output slots

    // Control pins shared across all bits
    Slot op_add;
    Slot op_and;
    Slot op_or;
    Slot op_xor;
} ALUNBit;

void alu_nbit_init(ALUNBit *alu, int N, Slot *A, Slot *B) {
    alu->N = N;
    alu->bits = malloc(sizeof(ALUBit) * N);
    alu->A = A;
    alu->B = B;
    alu->result = malloc(sizeof(Slot*) * N);

    // Initialize control pins to 0
    alu->op_add.value = SIG_0;
    alu->op_and.value = SIG_0;
    alu->op_or.value  = SIG_0;
    alu->op_xor.value = SIG_0;

    for (int i = 0; i < N; i++) {
        ALUBit *b = &alu->bits[i];

        // Share control pins
        b->op_add = &alu->op_add;
        b->op_and = &alu->op_and;
        b->op_or  = &alu->op_or;
        b->op_xor = &alu->op_xor;

        // First carry-in = 0
        Slot *cin = i == 0 ? &GND : &alu->bits[i-1].add.cout;

        alu_bit_init(b, &A[i], &B[i], cin);
    }
}

void alu_nbit_eval(ALUNBit *alu) {
    for (int i = 0; i < alu->N; i++) {
        alu_bit_eval(&alu->bits[i]);
        alu->result[i] = &alu->bits[i].result;
    }
}
