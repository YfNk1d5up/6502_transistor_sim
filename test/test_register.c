#include <stdio.h>
#include <stdlib.h>
#include "../register.h"
#include "../helpers.h"

void print_bus(const char *name, Slot *bus, int n) {
    printf("%s = ", name);
    for (int i = n - 1; i >= 0; i--) { // MSB first
        printf("%d", bus[i].value == SIG_1 ? 1 : 0);
    }
    printf("\n");
}

int main(void) {
    const int N = 8;

    Slot D[N];
    Slot Q[N];
    Slot Q_not[N];
    Slot CLK;

    // Init signals
    for (int i = 0; i < N; i++) {
        D[i].value = SIG_0;
        Q[i].value = SIG_0;
        Q_not[i].value = SIG_1;
    }
    CLK.value = SIG_0;

    NBitRegister reg;
    nreg_init(&reg, N, D, Q, &CLK);

    printf("=== 8-bit register test ===\n");

    // ---- Cycle 0: load 10101010 ----
    D[0].value = SIG_0;
    D[1].value = SIG_1;
    D[2].value = SIG_0;
    D[3].value = SIG_1;
    D[4].value = SIG_0;
    D[5].value = SIG_1;
    D[6].value = SIG_0;
    D[7].value = SIG_1;

    CLK.value = SIG_1;   // enable
    nreg_eval(&reg);     // one cycle
    CLK.value = SIG_0;   // disable

    print_bus("Q after load 10101010", Q, N);

    // ---- Cycle 1: change D, hold ----
    D[0].value = SIG_1;
    D[1].value = SIG_1;
    D[2].value = SIG_1;
    D[3].value = SIG_1;
    D[4].value = SIG_1;
    D[5].value = SIG_1;
    D[6].value = SIG_1;
    D[7].value = SIG_1;

    CLK.value = SIG_0;   // hold
    nreg_eval(&reg);

    print_bus("Q after hold", Q, N);

    // ---- Cycle 2: load 01100110 ----
    D[0].value = SIG_0;
    D[1].value = SIG_1;
    D[2].value = SIG_1;
    D[3].value = SIG_0;
    D[4].value = SIG_0;
    D[5].value = SIG_1;
    D[6].value = SIG_1;
    D[7].value = SIG_0;

    CLK.value = SIG_1;   // enable
    nreg_eval(&reg);
    CLK.value = SIG_0;

    print_bus("Q after load 01100110", Q, N);

    return 0;
}
