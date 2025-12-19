#include <stdio.h>
#include <stdlib.h>
#include "../cpu.h"

// Convert Slot to string
const char* S(Slot s) {
    switch (s.value) {
        case SIG_0: return "0";
        case SIG_1: return "1";
        case SIG_Z: return "Z";
    }
    return "?";
}

// Print N-bit value
void print_slot_array(Slot *arr, int N) {
    for (int i = N-1; i >= 0; i--) {
        printf("%s", S(arr[i]));
    }
}

int main() {
    int N = 4; // 4-bit CPU for testing
    CPU_NBit cpu;

    // Initialize CPU
    cpu_nbit_init(&cpu, N);

    printf("N-bit CPU test\n");
    printf("regA  regB  | ALU_OUT regOut\n");
    printf("----------------------------\n");

    // Test all combinations of A and B for 4-bit
    for (int a = 0; a < 16; a++) {
        for (int b = 0; b < 16; b++) {
            // Load regA
            for (int i = 0; i < N; i++)
                cpu.regA.D[i].value = (a & (1 << i)) ? SIG_1 : SIG_0;

            // Load regB
            for (int i = 0; i < N; i++)
                cpu.regB.D[i].value = (b & (1 << i)) ? SIG_1 : SIG_0;

            // Set clock high to write inputs
            cpu.clk.value = SIG_1;
            register_nbit_eval(&cpu.regA);
            register_nbit_eval(&cpu.regB);

            // --- Test ALU ADD ---
            cpu.op_add.value = SIG_1;
            cpu.op_and.value = SIG_0;
            cpu.op_or.value  = SIG_0;
            cpu.op_xor.value = SIG_0;
            cpu.weOut.value  = SIG_1; // write result
            cpu_nbit_eval(&cpu);

            printf(" ");
            print_slot_array(cpu.regA.Q, N);
            printf(" ");
            print_slot_array(cpu.regB.Q, N);
            printf(" | ");
            print_slot_array(cpu.alu.result, N);
            printf(" ");
            print_slot_array(cpu.regOut.Q, N);
            printf("\n");

            // Reset write-enable
            cpu.weOut.value = SIG_0;
        }
    }

    return 0;
}
