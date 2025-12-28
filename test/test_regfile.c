#include <stdio.h>
#include <stdlib.h>
#include "../regfile.h"
#include "../regalu.h"
#include "../helpers.h"


static void print_register_port(const char *name, NBitRegister *reg, int port, int N) {
    printf("%s (Q):\n", name);

    for (int i = 0; i < N; i++) {
            printf("%s ", S(reg->enableports[port].Q[i].value));
            //printf("bits %s \n", S(reg->bits[i].Q.value));
    }
    printf("\n");
}

static void print_tristate(const char *name, NBitRegister *reg, int port, int N) {
    printf("%s (Q):\n", name);

    for (int i = 0; i < N; i++) {
            printf("%s", S(reg->enableports[port].Q[i].value));
            print_bus("tsQ", &reg->enableports[port].tsQ[i].out, 1);
    }
    printf("\n");
}


static void dump(RegFile *rf, ProgramCounter *pc, RegALU *alu) {

    /* ---- Register output drivers ---- */
    /*
    print_slots("ALU Core A", alu->core.A,   rf->N);
    print_slots("ALU Core B", alu->core.B,   rf->N);

    print_slots("AC_DB  ", rf->AC_DB,   rf->N);
    print_slots("AC_SB  ", rf->AC_SB,   rf->N);

    print_slots("X_SB   ", rf->X_SB,    rf->N);
    print_slots("Y_SB   ", rf->Y_SB,    rf->N);

    print_slots("SP_SB  ", rf->SP_SB,   rf->N);
    print_slots("SP_ADL ", rf->SP_ADL,  rf->N);

    print_slots("P_DB   ", rf->P_DB,    rf->N);

    print_slots("PCL_DB ", rf->PCL_DB,  rf->N);
    print_slots("PCL_ADL", rf->PCL_ADL, rf->N);

    print_slots("PCH_DB ", rf->PCH_DB,  rf->N);
    print_slots("PCH_ADH", rf->PCH_ADH, rf->N);
    */
    /* ---- Resolved buses ---- */
    print_bus("DB ",  rf->dataBus,     rf->N);
    print_bus("SB ",  rf->stackBus,    rf->N);
    print_bus("ADL",  rf->addressLBus, rf->N);
    print_bus("ADH",  rf->addressHBus, rf->N);

    printf("------------------------------------------------\n");
}


static void bus_drive(Slot *in, const int *v, int N) {
    for (int i = 0; i < N; i++)
        in[i].value = v[i];
}

static void bus_release(Slot *in, int N) {
    for (int i = 0; i < N; i++)
        in[i].value = SIG_Z;
}

void multi_eval (RegFile *rf, ProgramCounter *pc, RegALU *alu) {
    printf("    CLOCK 0   \n");
    rf->CLK->value = SIG_0;
    for (int i=0; i < 10; i++)
        regfile_eval(rf, pc, alu);    
    dump(rf, pc, alu);
    printf("    CLOCK 1   \n");
    rf->CLK->value = SIG_1;
    for (int i=0; i < 10; i++)
        regfile_eval(rf, pc, alu);
    dump(rf, pc, alu);
    printf("    CLOCK 0   \n");
    rf->CLK->value = SIG_0;
    for (int i=0; i < 10; i++)
        regfile_eval(rf, pc, alu);    
    dump(rf, pc, alu);

}

/* ---------------- Test ---------------- */

int main(void) {
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

    RegFile rf;
    RegFileEn en;
    ProgramCounter pc;
    RegALU alu;

    /* -------- Allocate buses -------- */
    Node *DB  = malloc(sizeof(Node) * N);
    Node *SB  = malloc(sizeof(Node) * N);
    Node *ADL = malloc(sizeof(Node) * N);
    Node *ADH = malloc(sizeof(Node) * N);

    allocate_node(DB,  4, N);
    allocate_node(SB,  5, N);
    allocate_node(ADL, 3, N);
    allocate_node(ADH, 1, N);

    /* -------- Input bus drivers -------- */
    Slot *DB_IN  = malloc(sizeof(Slot) * N);
    Slot *SB_IN  = malloc(sizeof(Slot) * N);
    Slot *ADL_IN = malloc(sizeof(Slot) * N);
    Slot *ADH_IN = malloc(sizeof(Slot) * N);

    for (int i = 0; i < N; i++) {
        DB_IN[i].value  = SIG_Z;
        SB_IN[i].value  = SIG_Z;
        ADL_IN[i].value = SIG_Z;
        ADH_IN[i].value = SIG_Z;

        node_add_slot(&DB[i],  &DB_IN[i]);
        node_add_slot(&SB[i],  &SB_IN[i]);
        node_add_slot(&ADL[i], &ADL_IN[i]);
        node_add_slot(&ADH[i], &ADH_IN[i]);
    }

    /* -------- Allocate control signals -------- */
    #define ALLOC(x) x = malloc(sizeof(Slot))

    ALLOC(en.one);
    ALLOC(en.zero);

    ALLOC(en.LOAD_0_ADD);
    ALLOC(en.LOAD_ADL_ADD);
    ALLOC(en.LOAD_DB_ADD);
    ALLOC(en.LOAD_SB_ADD);
    ALLOC(en.LOAD_notDB_ADD);
    ALLOC(en.LOAD_SB_AC);
    ALLOC(en.LOAD_SB_X);
    ALLOC(en.LOAD_SB_Y);
    ALLOC(en.LOAD_SB_SP);
    ALLOC(en.LOAD_DB_P);
    ALLOC(en.LOAD_ADL_PCL);
    ALLOC(en.LOAD_PCL_PCL);
    ALLOC(en.LOAD_ADH_PCH);
    ALLOC(en.LOAD_PCH_PCH);

    ALLOC(en.EN_ADD_ADL);
    ALLOC(en.EN_ADD06_SB);
    ALLOC(en.EN_ADD7_SB);
    ALLOC(en.EN_AC_DB);
    ALLOC(en.EN_AC_SB);
    ALLOC(en.EN_X_SB);
    ALLOC(en.EN_Y_SB);
    ALLOC(en.EN_SP_SB);
    ALLOC(en.EN_SP_ADL);
    ALLOC(en.EN_P_DB);
    ALLOC(en.EN_PCL_DB);
    ALLOC(en.EN_PCL_ADL);
    ALLOC(en.EN_PCH_DB);
    ALLOC(en.EN_PCH_ADH);
    ALLOC(en.EN_I_PC);

    /* Default all control lines low */
    #define CLR(x) x->value = SIG_0
    CLR(en.LOAD_0_ADD);
    CLR(en.LOAD_ADL_ADD);
    CLR(en.LOAD_DB_ADD);
    CLR(en.LOAD_SB_ADD);
    CLR(en.LOAD_notDB_ADD);
    CLR(en.EN_ADD06_SB);
    CLR(en.EN_ADD7_SB);
    CLR(en.LOAD_SB_AC); CLR(en.LOAD_SB_X); CLR(en.LOAD_SB_Y);
    CLR(en.LOAD_SB_SP); CLR(en.LOAD_DB_P);
    CLR(en.LOAD_ADL_PCL); CLR(en.LOAD_PCL_PCL);
    CLR(en.LOAD_ADH_PCH); CLR(en.LOAD_PCH_PCH);
    CLR(en.EN_ADD06_SB);
    CLR(en.EN_ADD7_SB);
    CLR(en.EN_ADD_ADL);
    CLR(en.EN_AC_DB); CLR(en.EN_AC_SB);
    CLR(en.EN_X_SB);  CLR(en.EN_Y_SB);
    CLR(en.EN_SP_SB); CLR(en.EN_SP_ADL);
    CLR(en.EN_P_DB);
    CLR(en.EN_PCL_DB); CLR(en.EN_PCL_ADL);
    CLR(en.EN_PCH_DB); CLR(en.EN_PCH_ADH);
    CLR(en.EN_I_PC);

    en.one->value = SIG_1;

    /* -------- Init regfile -------- */
    regfile_init(&rf, N, &CLK, one, zero, dummy, DB, SB, ADL, ADH, &en);
    
    pc_init(&pc, 
        N, 
        &CLK, 
        one,
        zero,
        dummy, 
        &rf.regPCLS, 
        &rf.regPCHS, 
        &rf.regPCL, 
        &rf.regPCH,
        en.one, 
        en.zero, 
        en.LOAD_PCL_PCL,
        en.LOAD_PCH_PCH,
        en.EN_I_PC
    );
    
    regfile_connect2buses(&rf);

    alu_init(
        &alu, 
        N, 
        &CLK, 
        one, 
        zero, 
        dummy, 
        &rf.regA, 
        &rf.regB, 
        &rf.regAH, 
        en.one, 
        en.zero
    );

    

    /* -------- Test values -------- */
    int A[8]  = {1,0,1,0,1,0,1,0};
    int X[8]  = {0,1,0,1,0,1,0,1};
    int SP[8] = {1,1,0,0,1,1,0,0};
    int PCLS[8] = {1,1,1,1,1,1,1,1};
    int PCHS[8] = {1,0,0,0,0,0,0,0};

    /* -------- Load AC from SB -------- */
    // Looking at PCL on ADL at all cycles
    en.LOAD_PCL_PCL->value = SIG_1;
    en.EN_PCL_ADL->value = SIG_1;

    // Looking at PCH on ADH at all cycles
    en.LOAD_PCH_PCH->value = SIG_1;
    en.EN_PCH_ADH->value = SIG_1;
    
    en.EN_I_PC->value = SIG_1;
    
    // initialization issue without first eval
    printf("Inititial state buses\n");
    multi_eval(&rf, &pc, &alu);

    printf("Load AC from Stack Bus\n");
    bus_drive(SB_IN, A, N);
    en.LOAD_SB_AC->value = SIG_1;

    multi_eval(&rf, &pc, &alu);

    en.LOAD_SB_AC->value = SIG_0;
    bus_release(SB_IN, N);

    multi_eval(&rf, &pc, &alu);

    /* -------- Load X from SB -------- */
    printf("Load X from Stack Bus\n");
    bus_drive(SB_IN, X, N);
    en.LOAD_SB_X->value = SIG_1;

    multi_eval(&rf, &pc, &alu);

    en.LOAD_SB_X->value = SIG_0;
    bus_release(SB_IN, N);

    multi_eval(&rf, &pc, &alu);

    /* -------- Enable AC → DB -------- */
    printf("Enable AC onto Data Bus\n");
    en.EN_AC_DB->value = SIG_1;

    multi_eval(&rf, &pc, &alu);

    printf("Enable X onto Stack Bus (keeping AC on DB)\n");
    en.EN_X_SB->value = SIG_1;
        
    multi_eval(&rf, &pc, &alu);

    en.EN_X_SB->value = SIG_0;

    /* -------- jump counter -------- */
    printf("Loading to PLCS & PCHS to jump counter\n");
    en.EN_PCL_ADL->value = SIG_0;
    en.EN_PCH_ADH->value = SIG_0;
    bus_drive(ADL_IN, PCLS, N);
    bus_drive(ADH_IN, PCHS, N);
    en.LOAD_PCL_PCL->value = SIG_0;
    en.LOAD_PCH_PCH->value = SIG_0;
    en.EN_I_PC->value = SIG_0;
    en.LOAD_ADL_PCL->value = SIG_1;
    en.LOAD_ADH_PCH->value = SIG_1;

    multi_eval(&rf, &pc, &alu);

    printf("Restart counter\n");
    bus_release(ADL_IN, N);
    bus_release(ADH_IN, N);
    en.LOAD_ADL_PCL->value = SIG_0;
    en.LOAD_ADH_PCH->value = SIG_0;
    en.EN_PCL_ADL->value = SIG_1;
    en.EN_PCH_ADH->value = SIG_1;
    en.LOAD_PCL_PCL->value = SIG_1; 
    en.LOAD_PCH_PCH->value = SIG_1;
    en.EN_I_PC->value = SIG_1;

    multi_eval(&rf, &pc, &alu);

    /* -------- Load SP then drive ADL -------- */
    printf("Load SP and drive SB\n");
    bus_drive(SB_IN, SP, N);
    en.LOAD_SB_SP->value = SIG_1;

    multi_eval(&rf, &pc, &alu);

    printf("Load SB from SP\n");
    en.LOAD_SB_SP->value = SIG_0;
    bus_release(SB_IN, N);
    en.EN_SP_SB->value = SIG_1;
    
    multi_eval(&rf, &pc, &alu);

    /* -------- Load ALU B from DB and ALU A from SB -------- */
    printf("Load ALU B from DB and ALU A from SB and operation = add\n");
    en.LOAD_DB_ADD->value = SIG_1;
    en.EN_SP_SB->value = SIG_1;
    en.LOAD_SB_ADD->value = SIG_1;
    alu.core.op_add.value = SIG_1; 

    multi_eval(&rf, &pc, &alu);

    /* -------- Output Result from ALU to SB -------- */
    printf("Output Result [0,6] from ALU to SB\n");
    en.LOAD_DB_ADD->value = SIG_0;
    en.LOAD_SB_ADD->value = SIG_0;
    en.EN_SP_SB->value = SIG_0;
    en.EN_ADD06_SB->value = SIG_1;
    //en.EN_ADD7_SB->value = SIG_1;

    multi_eval(&rf, &pc, &alu);

    return 0;
}
