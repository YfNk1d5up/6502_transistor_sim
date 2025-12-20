#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "transistor.h"
#include "regfile.h"
#include "regalu.h"

// ================= CPU =================

typedef struct {
    // Clock
    Slot CLK;

    // External input bus
    Slot inputD[8];

    // Shared system bus
    Node bus[8];
    Slot *busSlots[8];

    // Register file
    RegFile rf;
    RegFileEn rf_en;

    // Registered ALU
    RegALU alu;
    RegAluEn alu_en;
} CPU;

// ================= Debug =================

static unsigned read_slots(const Slot *s, int N)
{
    unsigned v = 0;
    for (int i = 0; i < N; i++)
        if (s[i].value == SIG_1)
            v |= (1u << i);
    return v;
}

static unsigned read_slots_ptr(Slot **s, int N)
{
    unsigned v = 0;
    for (int i = 0; i < N; i++)
        if (s[i] && s[i]->value == SIG_1)
            v |= (1u << i);
    return v;
}

static unsigned read_nodes(const Node *n)
{
    unsigned v = 0;
    for (int i = 0; i < 8; i++)
        if (n[i].resolved.value == SIG_1)
            v |= (1u << i);
    return v;
}

static unsigned read_regQ(const NBitRegister *r)
{
    unsigned v = 0;
    for (int i = 0; i < 8; i++)
        if (r->Q[i].value == SIG_1)
            v |= (1u << i);
    return v;
}

static void debug_dump(CPU *cpu, const char *tag)
{
    printf("\n--- %s ---\n", tag);
    printf("CLK  = %d\n", cpu->CLK.value);
    printf("PCL  = 0x%02X\n", read_regQ(&cpu->rf.regPCL));
    printf("PCH  = 0x%02X\n", read_regQ(&cpu->rf.regPCH));
    printf("BUS  = 0x%02X\n", read_nodes(cpu->bus));
    printf("ALU A Reg = 0x%02X\n", read_slots(cpu->alu.A, 8));
    printf("ALU B Reg = 0x%02X\n", read_slots(cpu->alu.B, 8));
    printf("ALU OUT = 0x%02X\n", read_slots_ptr(cpu->alu.core.result, 8));
    printf("Bus slots:\n");
    for (int i = 0; i < 8; i++) {
        printf("  bit %d:", i);
        for (int j = 0; j < cpu->bus[i].n_slots; j++) {
            Slot *s = cpu->bus[i].slots[j];
            char val = 'X';
            if (s != NULL) {
                val = (s->value == SIG_0) ? '0' : (s->value == SIG_1) ? '1' : 'Z';
            }
            printf(" %c", val);
        }
        printf("\n");
    }
}

// ================= CPU INIT =================

void cpu_init(CPU *cpu)
{
    memset(cpu, 0, sizeof(*cpu));
    cpu->CLK.value = SIG_0;

    // ---- Bus + external input ----
    for (int i = 0; i < 8; i++) {
        cpu->inputD[i].value = SIG_Z;

        cpu->bus[i].n_slots = 0;
        cpu->bus[i].capacity = 0;
        cpu->bus[i].slots = NULL;

        node_add_slot(&cpu->bus[i], &cpu->inputD[i]);
    }

    // Connect bus slots
    for (int i = 0; i < 8; i++)
        cpu->busSlots[i] = &cpu->bus[i].resolved;

    // ---- Register file ----
    regfile_init(
        &cpu->rf,
        8,
        &cpu->CLK,
        cpu->busSlots,
        &cpu->rf_en
    );

    // Connect regfile outputs to bus
    for (int i = 0; i < 8; i++)
        node_add_slot(&cpu->bus[i], &cpu->rf.outputQ[i].resolved);

    // ---- ALU enables ----
    cpu->alu_en.EN_A.value   = SIG_0;
    cpu->alu_en.EN_B.value   = SIG_0;
    cpu->alu_en.EN_OUT.value = SIG_0;

    // ---- Registered ALU ----
    alu_init(
        &cpu->alu,
        8,
        &cpu->CLK,
        cpu->busSlots,
        &cpu->alu_en
    );

    // Connect ALU output to bus
    for (int i = 0; i < 8; i++)
        node_add_slot(&cpu->bus[i], cpu->alu.outputQ[i]);
}

// ================= CPU TICK =================

void cpu_tick(CPU *cpu)
{

    for (int i = 0; i < 8; i++)
        node_resolve(&cpu->bus[i]);

    regfile_eval(&cpu->rf);

    for (int i = 0; i < 8; i++)
        node_resolve(&cpu->bus[i]);

    alu_eval(&cpu->alu);

    for (int i = 0; i < 8; i++)
        node_resolve(&cpu->bus[i]);


    cpu->CLK.value = SIG_1;
    regfile_eval(&cpu->rf);

    for (int i = 0; i < 8; i++)
        node_resolve(&cpu->bus[i]);

    alu_eval(&cpu->alu);

    for (int i = 0; i < 8; i++)
        node_resolve(&cpu->bus[i]);

    cpu->CLK.value = SIG_0;
    regfile_eval(&cpu->rf);

    for (int i = 0; i < 8; i++)
        node_resolve(&cpu->bus[i]);

    alu_eval(&cpu->alu);

    for (int i = 0; i < 8; i++)
        node_resolve(&cpu->bus[i]);

    //debug_dump(cpu, "After falling edge");
}

// ================= MAIN =================

int main(void)
{
    CPU cpu;
    cpu_init(&cpu);

    // ---- Reset PC = 1 ----
    cpu.rf_en.EN_PCL.value = SIG_1;
    cpu.rf_en.EN_OUT.value = SIG_0;
    cpu.alu_en.EN_OUT.value = SIG_0;
    cpu.inputD[0].value = SIG_1;
    for (int i = 1; i < 8; i++)
        cpu.inputD[i].value = SIG_0;

    cpu_tick(&cpu);

    cpu.rf_en.EN_PCL.value = SIG_0;
    for (int i = 0; i < 8; i++)
        cpu.inputD[i].value = SIG_Z;

    // ---- PC increment cycle ----
    for (int cycle = 0; cycle < pow(2, 16) - 2; cycle++) {
        
        // 1) PC -> ALU A    
        cpu.rf_en.EN_OUT.value = SIG_1;
        cpu.rf_en.EN_PCL.value = SIG_1;
        cpu.alu_en.EN_A.value = SIG_1;

        cpu_tick(&cpu);
        cpu.alu_en.EN_A.value = SIG_0;
        cpu.rf_en.EN_PCL.value = SIG_0;
        cpu.rf_en.EN_OUT.value = SIG_0;

        // 2) Constant 1 -> ALU B
        cpu.inputD[0].value = SIG_1;
        for (int i = 1; i < 8; i++)
            cpu.inputD[i].value = SIG_0;

        cpu.alu_en.EN_B.value = SIG_1;
        cpu.alu.core.op_add.value = SIG_1;
        cpu.alu.core.op_sub.value = SIG_0;
        
        cpu_tick(&cpu);
        cpu.alu_en.EN_B.value = SIG_0;

        for (int i = 0; i < 8; i++)
            cpu.inputD[i].value = SIG_Z;

        // 3) ALU -> bus
        cpu.alu_en.EN_OUT.value = SIG_1;
        cpu.rf_en.EN_PCL.value = SIG_1;
        cpu.rf_en.EN_OUT.value = SIG_0;
        cpu_tick(&cpu);

        cpu.rf_en.EN_PCL.value = SIG_0;
        cpu.alu_en.EN_OUT.value = SIG_0;

        unsigned pc =
            (read_regQ(&cpu.rf.regPCH) << 8) |
            read_regQ(&cpu.rf.regPCL);

        printf("\nFINAL PC = 0x%04X\n", pc);
    }
    return 0;
}
