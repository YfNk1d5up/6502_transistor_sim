#include <stdlib.h>

#include "cpu.h"
#include "sram.h"

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

    cpu->outAddressBusL = malloc(sizeof(Slot) * N);
    cpu->outAddressBusH = malloc(sizeof(Slot) * N);
    cpu->outDataBus = malloc(sizeof(Slot) * N);

    // Clear storage
    for (int i = 0; i < N; i++) {
        cpu->outAddressBusL[i].value  = SIG_Z;
        cpu->outAddressBusH[i].value  = SIG_Z;
        cpu->outDataBus[i].value  = SIG_Z;        
    }

    allocate_node(cpu->dataBus,  1, N);
    allocate_node(cpu->stackBus,  1, N);
    allocate_node(cpu->addressLBus, 1, N);
    allocate_node(cpu->addressHBus, 1, N);

    rcl_init(&cpu->rcl);
    cpu->RnotW = cpu->rcl.RnotW;

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

    nreg_init(
        &cpu->ABL, 
        cpu->N,
        1,
        1,
        cpu->clkGen.phi1
    );
    nreg_add_load_port(&cpu->ABL, 0, cpu->rf.addressLBusD, cpu->rcl.LOAD_ADL_ABL);  
    nreg_add_enable_port(&cpu->ABL, 0, cpu->outAddressBusL, cpu->dummy, cpu->one[0]);
    
    nreg_init(
        &cpu->ABH, 
        cpu->N,
        1,
        1,
        cpu->clkGen.phi1
    );
    nreg_add_load_port(&cpu->ABH, 0, cpu->rf.addressHBusD, cpu->rcl.LOAD_ADH_ABH);  
    nreg_add_enable_port(&cpu->ABH, 0, cpu->outAddressBusH, cpu->dummy, cpu->one[0]);

    cpu->outAddressBus = malloc(sizeof(Slot*) * 2*cpu->N);
    for (int i = 0; i < cpu->N; i++) {
        cpu->outAddressBus[i] = &cpu->outAddressBusL[i];
        cpu->outAddressBus[cpu->N + i] = &cpu->outAddressBusH[i];
    }

    nreg_init(
        &cpu->DOR, 
        cpu->N,
        1,
        1,
        cpu->clkGen.phi1
    );
    nreg_add_load_port(&cpu->DOR, 0, cpu->rf.dataBusD, cpu->one[0]);  
    nreg_add_enable_port(&cpu->DOR, 0, cpu->outDataBus, cpu->dummy, cpu->one[0]);

    cpu->enOutBuffers_not = malloc(sizeof(NOTGate) * cpu->N);
    cpu->enOutBuffers_and = malloc(sizeof(ANDGate) * cpu->N);
    cpu->extDataBusTristate = malloc(sizeof(TriStateGate) * cpu->N);
    cpu->extDataBus = malloc(sizeof(Slot*) * N);
    for (int i = 0; i < cpu->N; i++) {
        not_init(&cpu->enOutBuffers_not[i], cpu->RnotW);
        and_init(
            &cpu->enOutBuffers_and[i], 
            cpu->clkGen.phi2, 
            &cpu->enOutBuffers_not[i].out.resolved
        );
        tristate_init(
            &cpu->extDataBusTristate[i], 
            &cpu->outDataBus[i], 
            &cpu->enOutBuffers_and[i].out.resolved
        );
        cpu->extDataBus[i] = &cpu->extDataBusTristate[i].out.resolved;
    }

    timing_init(&cpu->tgl, cpu->clkGen.phi1); // bypass predecode for now

    nreg_init(
        &cpu->IR, 
        cpu->N,
        1,
        1,
        cpu->clkGen.phi1 // bypass predecode for now
    );

    // bypass predecode for now
    cpu->IR_IN  = malloc(sizeof(Slot*) * N);
    for (int i = 0; i < N; i++)
        cpu->IR_IN[i] = malloc(sizeof(Slot));
    cpu->IR_OUT  = malloc(sizeof(Slot) * N);
    cpu->TGL_OUT  = malloc(sizeof(Slot) * N);
    nreg_add_load_port(&cpu->IR, 0, cpu->IR_IN, one[0]);  
    nreg_add_enable_port(&cpu->IR, 0, cpu->IR_OUT, cpu->dummy, one[0]);

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

void out_eval(CPU *cpu) {
    for (int i=0; i < cpu->N; i++) {
        not_eval(&cpu->enOutBuffers_not[i]);
        and_eval(&cpu->enOutBuffers_and[i]);
        tristate_eval(&cpu->extDataBusTristate[i]);
    }
}

void simple_eval(CPU *cpu) {
    clock_eval(&cpu->clkGen);
    for (int i=0; i < 10; i++) {
        nreg_eval(&cpu->IR);
        regfile_eval(&cpu->rf, &cpu->pc, &cpu->alu); 
        nreg_eval(&cpu->DOR);
        out_eval(cpu);
    } 
    dump_buses(&cpu->rf);
    //print_slots_ptr("TGL", cpu->tgl.out, 7);
}

void multi_eval(CPU *cpu, Slot *CLK) {
    CLK->value = SIG_0;
    printf("    CLOCK 0   \n");
    simple_eval(cpu);
    printf("    CLOCK 1   \n");
    CLK->value = SIG_1;
    simple_eval(cpu);
    printf("    CLOCK 0   \n");
    CLK->value = SIG_0;
    simple_eval(cpu);
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

    SRAM ram;
    sram_init(
        &ram, 
        N,
        &CLK, 
        dummy,
        one,
        zero,
        16,
        cpu.RnotW,
        cpu.extDataBus,
        cpu.outAddressBus
    );

    hex_to_slots_ptr(0xF0, cpu.IR_IN, N);
    hex_to_slots(0x80, cpu.TGL_OUT, N);

    printf("Inititial state buses\n");

    multi_eval(&cpu, &CLK);
  
    //timing_eval(&cpu.tgl); not working now, need precharge and two clocks
    rcl_eval(&cpu.rcl, cpu.IR_OUT, cpu.TGL_OUT); // should be in simple eval then
    
    hex_to_slots_ptr(0xF0, cpu.IR_IN, N);
    hex_to_slots(0x40, cpu.TGL_OUT, N);

    printf("After micro code 0xF080\n");
    multi_eval(&cpu, &CLK);

    rcl_eval(&cpu.rcl, cpu.IR_OUT, cpu.TGL_OUT); // should be in simple eval then

    printf("After micro code 0xF40\n");
    multi_eval(&cpu, &CLK);


}
