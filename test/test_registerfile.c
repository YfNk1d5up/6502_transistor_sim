#include <stdio.h>
#include <stdlib.h>
#include "../regfile.h"
#include "../register.h"
#include "../gates.h"

#define R 4
#define N 4


// Helper: print N-bit bus
void print_bus(Slot *bus, int N_BITS) {
    for (int i = N_BITS-1; i >= 0; i--) {
        printf("%d", bus[i].value);
    }
    printf("\n");
}

int main(void) {

    Slot data_in[N];
    Slot data_out[N];
    Slot write_sel[R];
    Slot read_sel[R];
    Slot clk;

    // Initialize signals
    for (int i = 0; i < N; i++) {
        data_in[i].value = SIG_0;
        data_out[i].value = SIG_0;
    }
    for (int i = 0; i < R; i++) {
        write_sel[i].value = SIG_0;
        read_sel[i].value = SIG_0;
    }
    clk.value = SIG_0;

    // Initialize register file
    RegFileNBit rf;
    regfile_nbit_init(&rf, N, R, data_in, data_out, write_sel, read_sel, &clk);

    // Values to write into registers
    int write_vals[R] = {0b1010, 0b0101, 0b1111, 0b1111};

    // Write each value to the corresponding register using one-hot write select
    for (int r = 0; r < R; r++) {
        // Set data_in
        for (int b = 0; b < N; b++) {
            data_in[b].value = (write_vals[r] & (1 << b)) ? SIG_1 : SIG_0;
        }

        // Set write select
        for (int i = 0; i < R; i++)
            write_sel[i].value = (i == r) ? SIG_1 : SIG_0;

        // Rising clock edge
        clk.value = SIG_1;
        regfile_nbit_eval(&rf);
        clk.value = SIG_0; // falling edge
    }

    // Read and print all registers using one-hot read select
    printf("Register file contents:\n");
    for (int r = 0; r < R; r++) {
        // Set read select
        for (int i = 0; i < R; i++)
            read_sel[i].value = (i == r) ? SIG_1 : SIG_0;

        // Evaluate read bus
        regfile_nbit_eval(&rf);

        printf("Reg %d: ", r);
        print_bus(data_out, N);
    }

    return 0;
}
