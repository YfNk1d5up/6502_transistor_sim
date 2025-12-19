#include <stdio.h>
#include <stdlib.h>
#include "alu.h"

// Convert Slot to string for printing
const char* S(Slot s) {
    switch (s.value) {
        case SIG_0: return "0";
        case SIG_1: return "1";
        case SIG_Z: return "Z";
    }
    return "?";
}

int main(void) {
    Slot A, B;
    ALUBit alu;

    // Initialize 1-bit ALU
    A.value = SIG_0;
    B.value = SIG_0;
    // Control pins initialized to 0
    alu.op_add = malloc(sizeof(Slot));
    alu.op_and = malloc(sizeof(Slot));
    alu.op_or  = malloc(sizeof(Slot));
    alu.op_xor = malloc(sizeof(Slot));


    alu_bit_init(&alu, &A, &B, &GND); // Cin = 0

    printf("A B | SUM COUT AND OR XOR\n");
    printf("--------------------\n");

    // Test all input combinations
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            A.value = a ? SIG_1 : SIG_0;
            B.value = b ? SIG_1 : SIG_0;

            // Test each operation
            // --- ADD ---
            alu.op_add->value = SIG_1;
            alu.op_and->value = SIG_0;
            alu.op_or->value  = SIG_0;
            alu.op_xor->value = SIG_0;
            alu_bit_eval(&alu);
            Slot sum = alu.result;
            Slot cout = alu.add.cout;

            // --- AND ---
            alu.op_add->value = SIG_0;
            alu.op_and->value = SIG_1;
            alu.op_or->value  = SIG_0;
            alu.op_xor->value = SIG_0;
            alu_bit_eval(&alu);
            Slot and_out = alu.result;

            // --- OR ---
            alu.op_add->value = SIG_0;
            alu.op_and->value = SIG_0;
            alu.op_or->value  = SIG_1;
            alu.op_xor->value = SIG_0;
            alu_bit_eval(&alu);
            Slot or_out = alu.result;

            // --- XOR ---
            alu.op_add->value = SIG_0;
            alu.op_and->value = SIG_0;
            alu.op_or->value  = SIG_0;
            alu.op_xor->value = SIG_1;
            alu_bit_eval(&alu);
            Slot xor_out = alu.result;

            printf("%d %d |  %s   %s    %s   %s   %s\n",
                   a, b, S(sum), S(cout), S(and_out), S(or_out), S(xor_out));
        }
    }
    free(alu.op_add);
    free(alu.op_and);
    free(alu.op_or);
    free(alu.op_xor);
    return 0;
}
