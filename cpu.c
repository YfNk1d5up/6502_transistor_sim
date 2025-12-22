#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "transistor.h"
#include "regfile.h"
#include "regalu.h"
#include "gates.h"
#include "register.h"

// ================= CPU =================

typedef struct {
    
    // Register File 
    Slot LOAD_regfAC;
    Slot LOAD_regfX;
    Slot LOAD_regfY;
    Slot LOAD_regfSP;
    Slot LOAD_regfP;
    Slot LOAD_regfPC;
    Slot LOAD_regfPCL;
    Slot LOAD_regfPCH;

    Slot EN_regfAC;
    Slot EN_regfX;
    Slot EN_regfY;
    Slot EN_regfSP;
    Slot EN_regfP;
    Slot EN_regfPC;
    Slot EN_regfPCL;
    Slot EN_regfPCH;

    // ALU
    
    Slot LOAD_aluA;
    Slot LOAD_aluB;
    Slot EN_aluOUT;

    Slot LOAD_MAL;
    Slot EN_MAL;
} CpuCtl;

typedef struct {
    // Clock
    Slot CLK;

    // External input bus
    Slot inputD[8];

    // Data bus
    Node dataBus[8];
    Slot *dataBusSlots[8];

    // Address bus
    Node addressLBus[8];
    Slot *addressLBusSlots[8];

    Node addressHBus[8];
    Slot *addressHBusSlots[8];

    // Register file
    RegFile rf;
    RegFileEn rf_en;

    // Registered ALU
    RegALU alu;
    RegAluEn alu_en;

    // Memory Address Low Register
    Slot MAL[8];  
    ANDGate *andMAL;
    TriStateGate *tsMAL;
    NBitRegister MAL;

    // Memory Address High Register
    Slot MAH[8];  
    ANDGate *andMAH;
    TriStateGate *tsMAH;
    NBitRegister MAH; // Memory Address High Register

    NBitRegister IR;  // Instruction Register
    NBitRegister PS;  // Processor Status Register

    NBitRegister DOR; // Data Output Register

    Slot decodeRomInput[8];

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

static void debug_dump(CPU *cpu, CpuCtl *cpu_ctl, const char *tag)
{
    printf("\n--- %s ---\n", tag);
    printf("CLK  = %d\n", cpu->CLK.value);
    printf("EN_PCL = %d\n", cpu->rf_en.EN_PCL->value);
    printf("PCL  = 0x%02X\n", read_regQ(&cpu->rf.regPCL));
    printf("EN_PCH = %d\n", cpu->rf_en.EN_PCH->value);
    printf("PCH  = 0x%02X\n", read_regQ(&cpu->rf.regPCH));
    printf("BUS  = 0x%02X\n", read_nodes(cpu->dataBus));
    printf("ALU A Reg = 0x%02X\n", read_slots(cpu->alu.A, 8));
    printf("ALU B Reg = 0x%02X\n", read_slots(cpu->alu.B, 8));
    printf("ALU OUT = 0x%02X\n", read_slots_ptr(cpu->alu.core.result, 8));
    printf("ALU FLAGS = C %d Z %d N %d V %d\n", cpu->alu.core.flags.C, cpu->alu.core.flags.Z, cpu->alu.core.flags.N, cpu->alu.core.flags.V);
    printf("Bus slots:\n");
    for (int i = 0; i < 8; i++) {
        printf("  bit %d:", i);
        for (int j = 0; j < cpu->dataBus[i].n_slots; j++) {
            Slot *s = cpu->dataBus[i].slots[j];
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

void cpu_init(CPU *cpu, CpuCtl *cpu_ctl)
{
    memset(cpu, 0, sizeof(*cpu));
    cpu->CLK.value = SIG_0;

    // ---- Bus + external input ----
    for (int i = 0; i < 8; i++) {
        cpu->inputD[i].value = SIG_Z;

        cpu->dataBus[i].n_slots = 0;
        cpu->dataBus[i].capacity = 0;
        cpu->dataBus[i].slots = NULL;

        node_add_slot(&cpu->dataBus[i], &cpu->inputD[i]);
    }

    // Connect bus slots
    for (int i = 0; i < 8; i++)
        cpu->dataBusSlots[i] = &cpu->dataBus[i].resolved;

    // ---- ALU enables ----
    cpu->alu_en.EN_A = &cpu_ctl->LOAD_aluA;
    cpu->alu_en.EN_B = &cpu_ctl->LOAD_aluB;
    cpu->alu_en.EN_OUT = &cpu_ctl->EN_aluOUT;

    // ---- Registered ALU ----
    alu_init(
        &cpu->alu,
        8,
        &cpu->CLK,
        cpu->dataBusSlots,
        &cpu->alu_en
    );

    // Connect ALU output to data bus
    for (int i = 0; i < 8; i++)
        node_add_slot(&cpu->dataBus[i], cpu->alu.outputQ[i]);

    // ---- Register file ----

    // Enables
    cpu->rf_en.EN_AC = &cpu_ctl->LOAD_regfAC;
    cpu->rf_en.EN_X = &cpu_ctl->LOAD_regfX;
    cpu->rf_en.EN_Y = &cpu_ctl->LOAD_regfY;
    cpu->rf_en.EN_SP = &cpu_ctl->LOAD_regfSP;
    cpu->rf_en.EN_P = &cpu_ctl->LOAD_regfP;
    cpu->rf_en.EN_PC = &cpu_ctl->LOAD_regfPC;
    cpu->rf_en.EN_PCL = &cpu_ctl->LOAD_regfPCL;
    cpu->rf_en.EN_PCH = &cpu_ctl->LOAD_regfPCH;
    cpu->rf_en.EN_OUT = &cpu_ctl->EN_regfAC;
    cpu->rf_en.EN_X = &cpu_ctl->EN_regfX;
    cpu->rf_en.EN_Y = &cpu_ctl->EN_regfY;
    cpu->rf_en.EN_SP = &cpu_ctl->EN_regfSP;
    cpu->rf_en.EN_P = &cpu_ctl->EN_regfP;
    cpu->rf_en.EN_PC = &cpu_ctl->EN_regfPC;
    cpu->rf_en.EN_PCL = &cpu_ctl->EN_regfPCL;
    cpu->rf_en.EN_PCH = &cpu_ctl->EN_regfPCH;

    regfile_init(
        &cpu->rf,
        8,
        &cpu->CLK,
        cpu->dataBusSlots,
        &cpu->rf_en
    );

    // Connect regfile outputs to bus
    for (int i = 0; i < 8; i++)
        node_add_slot(&cpu->dataBus[i], &cpu->rf.outputQ[i].resolved);

    nreg_init(&cpu->MAL, 8, cpu->addressLBusSlots, );

    nreg_init(&cpu->IR, 8, cpu->dataBusSlots, cpu->decodeRomInput, &cpu->CLK);
    
}

// ================= CPU TICK =================

void cpu_tick(CPU *cpu)
{

    for (int i = 0; i < 8; i++)
        node_resolve(&cpu->dataBus[i]);

    alu_eval(&cpu->alu);

    for (int i = 0; i < 8; i++)
        node_resolve(&cpu->dataBus[i]);

    regfile_eval(&cpu->rf);

    for (int i = 0; i < 8; i++)
        node_resolve(&cpu->dataBus[i]);

    cpu->CLK.value = SIG_1;
    regfile_eval(&cpu->rf);

    for (int i = 0; i < 8; i++)
        node_resolve(&cpu->dataBus[i]);

    alu_eval(&cpu->alu);

    for (int i = 0; i < 8; i++)
        node_resolve(&cpu->dataBus[i]);

    cpu->CLK.value = SIG_0;
    regfile_eval(&cpu->rf);

    for (int i = 0; i < 8; i++)
        node_resolve(&cpu->dataBus[i]);

    alu_eval(&cpu->alu);

    for (int i = 0; i < 8; i++)
        node_resolve(&cpu->dataBus[i]);

}

// ================= MAIN =================

int main(void)
{
    CPU cpu;
    CpuEn cpu_en;

    cpu_en.EN_regfAC.value = SIG_0;
    cpu_en.EN_regfX.value = SIG_0;
    cpu_en.EN_regfY.value = SIG_0;
    cpu_en.EN_regfSP.value = SIG_0;
    cpu_en.EN_regfP.value = SIG_0;
    cpu_en.EN_regfPCL.value = SIG_0;
    cpu_en.EN_regfPCH.value = SIG_0;
    cpu_en.EN_regfOUT.value = SIG_0;
    cpu_en.EN_aluA.value = SIG_0;
    cpu_en.EN_aluB.value = SIG_0;
    cpu_en.EN_aluOUT.value = SIG_0;

    cpu.alu.core.op_add.value = SIG_0;
    cpu.alu.core.op_sub.value = SIG_0;

    cpu_init(&cpu, &cpu_en);


    // ---- Reset PC ----

    cpu_en.EN_regfPCL.value = SIG_1;
    cpu_en.EN_regfPCH.value = SIG_0;
    cpu_en.EN_regfOUT.value = SIG_0;
    cpu_en.EN_aluA.value = SIG_0;
    cpu_en.EN_aluB.value = SIG_0;
    cpu_en.EN_aluOUT.value = SIG_0;


    for (int i = 0; i < 8; i++)
        cpu.inputD[i].value = SIG_1;
    //cpu.inputD[0].value = SIG_1;

    cpu_tick(&cpu);
    debug_dump(&cpu, &cpu_en, "Initialization");

    cpu_en.EN_regfPCL.value = SIG_0;
    cpu_en.EN_regfPCH.value = SIG_0;
    cpu_en.EN_regfOUT.value = SIG_0;
    cpu_en.EN_aluA.value = SIG_0;
    cpu_en.EN_aluB.value = SIG_0;
    cpu_en.EN_aluOUT.value = SIG_0;

    // shut off input slots
    for (int i = 0; i < 8; i++)
        cpu.inputD[i].value = SIG_Z;

    // ---- PC increment cycle ----
    for (int cycle = 0; cycle < 2; cycle++) { //pow(2, 16) - 2

        cpu.alu.core.op_add.value = SIG_0;
        
        // 1) PCL -> ALU A    

        cpu_en.EN_regfPCL.value = SIG_1;
        cpu_en.EN_regfPCH.value = SIG_0;
        cpu_en.EN_regfOUT.value = SIG_1;
        cpu_en.EN_aluA.value = SIG_1;
        cpu_en.EN_aluB.value = SIG_0;
        cpu_en.EN_aluOUT.value = SIG_0;

        cpu_tick(&cpu);
        debug_dump(&cpu, &cpu_en, "PCL -> ALU A");

        cpu_en.EN_aluA.value = SIG_0;
        cpu_en.EN_regfPCL.value = SIG_0;
        cpu_en.EN_regfOUT.value = SIG_0;

        // 2) Constant 1 -> ALU B

        cpu_en.EN_regfPCL.value = SIG_0;
        cpu_en.EN_regfPCH.value = SIG_0;
        cpu_en.EN_regfOUT.value = SIG_0;
        cpu_en.EN_aluA.value = SIG_0;
        cpu_en.EN_aluB.value = SIG_1;
        cpu_en.EN_aluOUT.value = SIG_0;

        // For now set constant 1 on input slots
        cpu.inputD[0].value = SIG_1;
        for (int i = 1; i < 8; i++)
            cpu.inputD[i].value = SIG_0;

        cpu.alu.core.op_add.value = SIG_1;
        
        cpu_tick(&cpu);
        debug_dump(&cpu, &cpu_en, "Constant 1 -> ALU B & increment");



        // 3) ALU -> PCL

        cpu_en.EN_regfPCL.value = SIG_1;
        cpu_en.EN_regfPCH.value = SIG_0;
        cpu_en.EN_regfOUT.value = SIG_0;
        cpu_en.EN_aluA.value = SIG_0;
        cpu_en.EN_aluB.value = SIG_0;
        cpu_en.EN_aluOUT.value = SIG_1;

        // shut off input slots
        for (int i = 0; i < 8; i++)
            cpu.inputD[i].value = SIG_Z;

        cpu_tick(&cpu);        
        debug_dump(&cpu, &cpu_en, "ALU RESULT -> PCL");

        // 4) If carry, read PCH

        cpu_en.EN_regfPCL.value = SIG_0;
        cpu_en.EN_regfPCH.value = SIG_1;
        cpu_en.EN_regfOUT.value = SIG_0;
        cpu_en.EN_aluA.value = SIG_0;
        cpu_en.EN_aluB.value = SIG_0;
        cpu_en.EN_aluOUT.value = SIG_0;

        cpu_tick(&cpu);
        debug_dump(&cpu, &cpu_en, "PCH READ IF CARRY");

        // 5) If carry, PCH -> ALU A
        cpu_en.EN_regfPCL.value = SIG_0;
        cpu_en.EN_regfPCH.value = SIG_1;
        cpu_en.EN_regfOUT.value = SIG_1;
        cpu_en.EN_aluA.value = SIG_1;
        cpu_en.EN_aluB.value = SIG_0;
        cpu_en.EN_aluOUT.value = SIG_0;

        cpu_tick(&cpu);
        debug_dump(&cpu, &cpu_en, "PCH READ IF CARRY");

        // 6) Constant 1 -> ALU B
        cpu_en.EN_regfPCL.value = SIG_0;
        cpu_en.EN_regfPCH.value = SIG_0;
        cpu_en.EN_regfOUT.value = SIG_0;
        cpu_en.EN_aluA.value = SIG_0;
        cpu_en.EN_aluB.value = SIG_1;
        cpu_en.EN_aluOUT.value = SIG_0;

        // For now set constant 1 on input slots
        cpu.inputD[0].value = SIG_1;
        for (int i = 1; i < 8; i++)
            cpu.inputD[i].value = SIG_0;

        cpu.alu.core.op_add.value = SIG_1;
        
        cpu_tick(&cpu);
        debug_dump(&cpu, &cpu_en, "Constant 1 -> ALU B & increment");



        // 7) ALU RESULT -> PCH

        cpu_en.EN_regfPCL.value = SIG_0;
        cpu_en.EN_regfPCH.value = SIG_1;
        cpu_en.EN_regfOUT.value = SIG_0;
        cpu_en.EN_aluA.value = SIG_0;
        cpu_en.EN_aluB.value = SIG_0;
        cpu_en.EN_aluOUT.value = SIG_1;

        // shut off input slots
        for (int i = 0; i < 8; i++)
            cpu.inputD[i].value = SIG_Z;

        cpu_tick(&cpu);        
        debug_dump(&cpu, &cpu_en, "If Carry, ALU RESULT -> PCH");

        unsigned pc =
            (read_regQ(&cpu.rf.regPCH) << 8) |
            read_regQ(&cpu.rf.regPCL);

        printf("\nFINAL PC = 0x%04X\n", pc);
    }
    return 0;
}
