#include "helpers.h"
#include "fulladder.h"
#include <stdio.h>
#include <math.h>

const char* S(Signal s) {
    switch(s) {
        case SIG_0: return "0";
        case SIG_1: return "1";
        case SIG_Z: return "Z";
    }
    return "?";
}

int main(void) {
    int N = 8; // 4-bit adder
    Slot A[N], B[N];

    // Prepare slots
    for (int i = 0; i < N; i++) {
        A[i].value = SIG_0;
        B[i].value = SIG_0;
    }

    FullAdderNBit adder;
    full_adder_nbit_init(&adder, A, B, N);

    printf("A   B   | SUM  COUT\n");
    printf("-------------------\n");

    // Test all combinations 0..15 for 4-bit inputs
    for (int a = 0; a < pow(2, N); a++) {
        for (int b = 0; b < pow(2, N); b++) {
            // Set bits for A and B
            for (int i = 0; i < N; i++) {
                A[i].value = (a >> i) & 1 ? SIG_1 : SIG_0;
                B[i].value = (b >> i) & 1 ? SIG_1 : SIG_0;
            }

            full_adder_nbit_eval(&adder);

            // Print inputs
            for (int i = N-1; i >= 0; i--) printf("%s", S(A[i].value));
            printf(" ");
            for (int i = N-1; i >= 0; i--) printf("%s", S(B[i].value));

            // Print sum
            printf(" | ");
            for (int i = N-1; i >= 0; i--) printf("%s", S(adder.sum[i]->value));

            // Print carry-out
            printf("  %s\n", S(adder.cout->value));
        }
    }

    return 0;
}
