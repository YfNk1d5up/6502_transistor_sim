#include <stdlib.h>
#include "fulladder.h"

// -------------------
// 1-bit Full Adder
// -------------------

// Initialize 1-bit full adder
void full_adder_init(FullAdder1Bit *fa, Slot *A, Slot *B, Slot *Cin) {
    xor_init(&fa->xor1, A, B);
    xor_init(&fa->xor2, &fa->xor1.out.resolved, Cin);

    and_init(&fa->and1, A, B);
    and_init(&fa->and2, Cin, &fa->xor1.out.resolved);

    or_init(&fa->or1, &fa->and1.out.resolved, &fa->and2.out.resolved);

    // Initialize sum and cout
    fa->sum.value = SIG_Z;
    fa->cout.value = SIG_Z;
}

// Evaluate 1-bit full adder
void full_adder_eval(FullAdder1Bit *fa) {
    xor_eval(&fa->xor1);
    xor_eval(&fa->xor2);

    and_eval(&fa->and1);
    and_eval(&fa->and2);

    or_eval(&fa->or1);

    // Final outputs
    fa->sum = fa->xor2.out.resolved;
    fa->cout = fa->or1.out.resolved;
}

// -------------------
// N-bit Full Adder
// -------------------


// Initialize N-bit full adder
void full_adder_nbit_init(FullAdderNBit *faN, Slot *A, Slot *B, int N) {
    faN->N = N;
    faN->bits = malloc(sizeof(FullAdder1Bit) * N);
    faN->sum = malloc(sizeof(Slot*) * N);

    static Slot zero = { SIG_0 };
    Slot *cin = &zero; // first carry-in is 0

    for (int i = 0; i < N; i++) {
        full_adder_init(&faN->bits[i], &A[i], &B[i], cin);
        faN->sum[i] = &faN->bits[i].sum;
        cin = &faN->bits[i].cout;
    }

    faN->cout = &faN->bits[N-1].cout;
}

// Evaluate N-bit full adder
void full_adder_nbit_eval(FullAdderNBit *faN) {
    for (int i = 0; i < faN->N; i++) {
        full_adder_eval(&faN->bits[i]);
    }
}
