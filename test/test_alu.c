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
    int N = 4;  // 4-bit ALU
    Slot A[4], B[4];
    ALUNBit alu;

    // Initialize input slots
    for (int i = 0; i < N; i++) {
        A[i].value = SIG_0;
        B[i].value = SIG_0;
    }

    // Initialize 4-bit ALU
    alu_nbit_init(&alu, N, A, B);

    printf("Testing 4-bit ALU\n");
    printf("A    B    | ADD   AND   OR    XOR\n");
    printf("--------------------------------\n");

    // Test all combinations of 4-bit inputs (0..15)
    for (int a = 0; a < (1 << N); a++) {
        for (int b = 0; b < (1 << N); b++) {
            // Set input slots
            for (int i = 0; i < N; i++) {
                A[i].value = (a & (1 << i)) ? SIG_1 : SIG_0;
                B[i].value = (b & (1 << i)) ? SIG_1 : SIG_0;
            }

            // --- ADD ---
            alu.op_add.value = SIG_1;
            alu.op_and.value = SIG_0;
            alu.op_or.value  = SIG_0;
            alu.op_xor.value = SIG_0;
            alu_nbit_eval(&alu);

            printf("%X    %X    | ", a, b);
            for (int i = N-1; i >= 0; i--) printf("%s", S(*alu.result[i]));
            printf("   ");

            // --- AND ---
            alu.op_add.value = SIG_0;
            alu.op_and.value = SIG_1;
            alu.op_or.value  = SIG_0;
            alu.op_xor.value = SIG_0;
            alu_nbit_eval(&alu);
            for (int i = N-1; i >= 0; i--) printf("%s", S(*alu.result[i]));
            printf("   ");

            // --- OR ---
            alu.op_add.value = SIG_0;
            alu.op_and.value = SIG_0;
            alu.op_or.value  = SIG_1;
            alu.op_xor.value = SIG_0;
            alu_nbit_eval(&alu);
            for (int i = N-1; i >= 0; i--) printf("%s", S(*alu.result[i]));
            printf("   ");

            // --- XOR ---
            alu.op_add.value = SIG_0;
            alu.op_and.value = SIG_0;
            alu.op_or.value  = SIG_0;
            alu.op_xor.value = SIG_1;
            alu_nbit_eval(&alu);
            for (int i = N-1; i >= 0; i--) printf("%s", S(*alu.result[i]));
            printf("\n");
        }
    }

    return 0;
}
