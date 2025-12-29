#include <stdlib.h>

#include "cpu.h"

/* ---------------- CPU ---------------- */

void cpu_init(
    CPU *cpu,
    int N, 
    Slot *CLK, 
    Slot **one, 
    Slot **zero,
    Slot *dummy
    )
{

    clock_init(&cpu->clkGen, CLK);
    cpu->N = N;
    cpu->one = one;
    cpu->zero = zero;
    cpu->dummy = dummy;

    /* -------- Allocate buses -------- */
    cpu->dataBus = malloc(sizeof(Node) * N);
    cpu->stackBus = malloc(sizeof(Node) * N);
    cpu->addressLBus = malloc(sizeof(Node) * N);
    cpu->addressHBus = malloc(sizeof(Node) * N);

    allocate_node(cpu->dataBus,  1, N);
    allocate_node(cpu->stackBus,  1, N);
    allocate_node(cpu->addressLBus, 1, N);
    allocate_node(cpu->addressHBus, 1, N);

    rcl_init(&cpu->rcl);

    /* -------- Init regfile -------- */
    regfile_init(
        &cpu->rf, 
        cpu->N, 
        cpu->clkGen.phi2, 
        cpu->one, 
        cpu->zero, 
        cpu->dummy, 
        cpu->dataBus, 
        cpu->stackBus,
        cpu->addressLBus,
        cpu->addressHBus, 
        cpu->rcl);
    
    pc_init(
        &cpu->pc, 
        cpu->N, 
        cpu->clkGen.phi2, 
        cpu->one,
        cpu->zero,
        cpu->dummy, 
        &cpu->rf.regPCLS, 
        &cpu->rf.regPCHS, 
        &cpu->rf.regPCL, 
        &cpu->rf.regPCH,
        cpu->one[0], 
        cpu->zero[0], 
        cpu->rcl
    );
    
    regfile_connect2buses(&cpu->rf);

    alu_init(
        &cpu->alu, 
        cpu->N, 
        cpu->clkGen.phi2, 
        cpu->one, 
        cpu->zero, 
        cpu->dummy, 
        &cpu->rf.regA, 
        &cpu->rf.regB, 
        &cpu->rf.regAH, 
        cpu->one[0], 
        cpu->zero[0]
    );

    timing_init(&cpu->tgl, cpu->clkGen.phi1); // bypass predecode for now

    nreg_init(
        &cpu->IR, 
        cpu->N,
        1,
        1,
        cpu->clkGen.phi1 // bypass predecode for now
    );

    // bypass predecode for now
    Slot **IR_IN  = malloc(sizeof(Slot*) * N);
    Slot *IR_OUT  = malloc(sizeof(Slot) * N);
    nreg_add_load_port(&cpu->IR, 0, IR_IN, one[0]);  
    nreg_add_enable_port(&cpu->IR, 0, IR_OUT, cpu->dummy, one[0]);

    cpu->decRom = &decodeRom;
}

static void bus_drive(Slot *in, const int *v, int N) {
    for (int i = 0; i < N; i++)
        in[i].value = v[i];
}

static void bus_release(Slot *in, int N) {
    for (int i = 0; i < N; i++)
        in[i].value = SIG_Z;
}

void multi_eval(CPU *cpu, Slot *CLK) {
    CLK->value = SIG_0;
    printf("    CLOCK 0   \n");
    clock_eval(&cpu->clkGen);
    for (int i=0; i < 10; i++)
        regfile_eval(&cpu->rf, &cpu->pc, &cpu->alu);    
    dump_buses(&cpu->rf);
    printf("    CLOCK 1   \n");
    CLK->value = SIG_1;
    clock_eval(&cpu->clkGen);
    for (int i=0; i < 10; i++)
        regfile_eval(&cpu->rf, &cpu->pc, &cpu->alu); 
    dump_buses(&cpu->rf);
    printf("    CLOCK 0   \n");
    CLK->value = SIG_0;
    clock_eval(&cpu->clkGen);
    for (int i=0; i < 10; i++)
        regfile_eval(&cpu->rf, &cpu->pc, &cpu->alu);    
    dump_buses(&cpu->rf);

}

int main() {
    
    const int N = 8;
    Slot CLK = { .value = SIG_0 };
    Slot **one = malloc(sizeof(Slot*) * N);
    Slot **zero = malloc(sizeof(Slot*) * N);
    for (int i = 0; i < N; i++) {
        one[i] = malloc(sizeof(Slot));
        one[i]->value = SIG_1;
        zero[i] = malloc(sizeof(Slot));
        zero[i]->value = SIG_0;
    }
    Slot *dummy = malloc(sizeof(Slot) * N);

    CPU cpu;
    cpu_init(
        &cpu,
        N,
        &CLK,
        one,
        zero,
        dummy
    );

    printf("Inititial state buses\n");
    multi_eval(&cpu, &CLK);
    uint16_t key = 0xF080;
    rcl_eval(&cpu.rcl, key);
    printf("After micro code 0xF080\n");
    multi_eval(&cpu, &CLK);


}
