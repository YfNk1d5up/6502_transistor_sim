#include "fulladder.h"
#include <stdio.h>

const char* S(Signal s) {
    switch(s) {
        case SIG_0: return "0";
        case SIG_1: return "1";
        case SIG_Z: return "Z";
    }
    return "?";
}

int main(void) {
    // 1-bit inputs
    Slot A, B, Cin;

    FullAdder1Bit fa;

    printf("A B Cin | Sum Cout\n");
    printf("-----------------\n");

    // Loop over all 8 possible combinations
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            for (int c = 0; c <= 1; c++) {
                A.value = a ? SIG_1 : SIG_0;
                B.value = b ? SIG_1 : SIG_0;
                Cin.value = c ? SIG_1 : SIG_0;

                // Initialize and evaluate full adder
                full_adder_init(&fa, &A, &B, &Cin);
                full_adder_eval(&fa);

                // Print result
                printf("%d %d  %d  |  %s   %s\n",
                    a, b, c, S(fa.sum.value), S(fa.cout.value));
            }
        }
    }

    return 0;
}
