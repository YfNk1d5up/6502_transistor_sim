#include <stdio.h>
#include <stdlib.h>
#include "../register.h"

// Convert Slot to string for printing
const char* S(Slot *s) {
    switch (s->value) {
        case SIG_0: return "0";
        case SIG_1: return "1";
        case SIG_Z: return "Z";
    }
    return "?";
}

void print_bus(const char *name, Slot *bus, int n) {
    printf("%s = ", name);
    for (int i = 0; i < n; i++) { // MSB first
        printf("%s", S(&bus[i]));
    }
    printf("\n");
}

int main(void) {
    const int N = 8;

    Slot *D[N];
    Slot Q[N];
    Slot Q_not[N];
    Slot CLK;
    Slot LOAD;
    Slot EN;

    // Init signals
    for (int i = 0; i < N; i++) {
        D[i] = malloc(sizeof(Slot));
        D[i]->value = SIG_0;
        Q[i].value = SIG_0;
        Q_not[i].value = SIG_1;
    }
    CLK.value = SIG_0;
    LOAD.value = SIG_0;
    EN.value = SIG_0;

    NBitRegister reg;
    nreg_init(&reg, N, D, Q, &LOAD, &EN, &CLK);
    printf("=== 8-bit register test ===\n");

    // ---- Cycle 0: load 10101010 ----
    D[0]->value = SIG_0;
    D[1]->value = SIG_1;
    D[2]->value = SIG_0;
    D[3]->value = SIG_1;
    D[4]->value = SIG_0;
    D[5]->value = SIG_1;
    D[6]->value = SIG_0;
    D[7]->value = SIG_1;

    CLK.value = SIG_1;   // enable
    nreg_eval(&reg);     // one cycle
    CLK.value = SIG_0;   // disable
    nreg_eval(&reg);     // one cycle
    print_bus("Q before load 10101010", Q, N);

    LOAD.value = SIG_1;
    CLK.value = SIG_1;   // enable
    nreg_eval(&reg);     // one cycle
    CLK.value = SIG_0;   // disable
    nreg_eval(&reg);     // one cycle
    print_bus("Q after load 10101010", Q, N);

    // ---- Cycle 1: change D, hold ----
    D[0]->value = SIG_1;
    D[1]->value = SIG_1;
    D[2]->value = SIG_1;
    D[3]->value = SIG_1;
    D[4]->value = SIG_1;
    D[5]->value = SIG_1;
    D[6]->value = SIG_1;
    D[7]->value = SIG_1;

    CLK.value = SIG_1;   // enable
    LOAD.value = SIG_0;
    EN.value = SIG_1;
    nreg_eval(&reg);     // one cycle
    CLK.value = SIG_0;   // disable
    nreg_eval(&reg);     // one cycle
    print_bus("Q before load with enable 11111111", Q, N);

    LOAD.value = SIG_1;
    CLK.value = SIG_1;   // enable
    nreg_eval(&reg);     // one cycle
    CLK.value = SIG_0;   // disable
    nreg_eval(&reg);     // one cycle
    print_bus("Q after load with enable 11111111", Q, N);

    EN.value = SIG_0;
    CLK.value = SIG_1;   // enable
    nreg_eval(&reg);     // one cycle
    CLK.value = SIG_0;   // disable
    nreg_eval(&reg);     // one cycle
    print_bus("Q after load without enable 11111111", Q, N);


    return 0;
}
