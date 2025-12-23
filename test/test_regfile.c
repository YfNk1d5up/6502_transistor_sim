#include <stdio.h>
#include <stdlib.h>
#include "../regfile.h"

/* ---------------- Helpers ---------------- */

static void print_bus(const char *name, Node *bus, int N) {
    printf("%s: ", name);
    for (int i = 0; i < N; i++)
        printf("%d ", bus[i].resolved.value);
    printf("\n");
}

static void print_slots(const char *name, Slot *s, int N) {
    printf("%s: ", name);
    for (int i = 0; i < N; i++)
        printf("%d ", s[i].value);
    printf("\n");
}

static void print_bus_slots(const char *name, Node *bus, int N) {
    printf("%s (all slots):\n", name);

    for (int bit = 0; bit < N; bit++) {
        printf("  [%d] ", bit);

        for (int s = 0; s < bus[bit].n_slots; s++) {
            printf("%d ", bus[bit].slots[s]->value);
        }

        printf("-> %d\n", bus[bit].resolved.value);
    }
}

static void dump(RegFile *rf) {

    /* ---- Register output drivers ---- */
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

/* ---------------- Test ---------------- */

int main(void) {
    const int N = 8;
    Slot CLK = { .value = SIG_0 };

    RegFile rf;
    RegFileEn en;

    /* -------- Allocate buses -------- */
    Node *DB  = malloc(sizeof(Node) * N);
    Node *SB  = malloc(sizeof(Node) * N);
    Node *ADL = malloc(sizeof(Node) * N);
    Node *ADH = malloc(sizeof(Node) * N);

    allocate_node(DB,  4, N);
    allocate_node(SB,  4, N);
    allocate_node(ADL, 2, N);
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

    ALLOC(en.LOAD_SB_AC);
    ALLOC(en.LOAD_SB_X);
    ALLOC(en.LOAD_SB_Y);
    ALLOC(en.LOAD_SB_SP);
    ALLOC(en.LOAD_DB_P);
    ALLOC(en.LOAD_ADL_PCL);
    ALLOC(en.LOAD_PCL_PCL);
    ALLOC(en.LOAD_ADH_PCH);
    ALLOC(en.LOAD_PCH_PCH);

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
    CLR(en.LOAD_SB_AC); CLR(en.LOAD_SB_X); CLR(en.LOAD_SB_Y);
    CLR(en.LOAD_SB_SP); CLR(en.LOAD_DB_P);
    CLR(en.LOAD_ADL_PCL); CLR(en.LOAD_PCL_PCL);
    CLR(en.LOAD_ADH_PCH); CLR(en.LOAD_PCH_PCH);
    CLR(en.EN_AC_DB); CLR(en.EN_AC_SB);
    CLR(en.EN_X_SB);  CLR(en.EN_Y_SB);
    CLR(en.EN_SP_SB); CLR(en.EN_SP_ADL);
    CLR(en.EN_P_DB);
    CLR(en.EN_PCL_DB); CLR(en.EN_PCL_ADL);
    CLR(en.EN_PCH_DB); CLR(en.EN_PCH_ADH);
    CLR(en.EN_I_PC);

    /* -------- Init regfile -------- */
    regfile_init(&rf, N, &CLK, DB, SB, ADL, ADH, &en);

    /* -------- Test values -------- */
    int A[8]  = {1,0,1,0,1,0,1,0};
    int X[8]  = {0,1,0,1,0,1,0,1};
    int SP[8] = {1,1,0,0,1,1,0,0};
    int PCLS[8] = {1,0,0,1,1,1,1,0};

    /* -------- Load AC from SB -------- */
    
    // initialization issue without first eval
    regfile_eval(&rf);

    // Looking at PCL on ADL at all cycles
    en.LOAD_PCL_PCL->value = SIG_1;
    en.EN_PCL_ADL->value = SIG_1;
    en.EN_I_PC->value = SIG_1;

    printf("Load AC from Stack Bus\n");
    bus_drive(SB_IN, A, N);
    en.LOAD_SB_AC->value = SIG_1;

    CLK.value = SIG_1;
    regfile_eval(&rf);
    CLK.value = SIG_0;
    dump(&rf);

    en.LOAD_SB_AC->value = SIG_0;
    bus_release(SB_IN, N);
    CLK.value = SIG_1;
    regfile_eval(&rf);
    CLK.value = SIG_0;

    dump(&rf);

    /* -------- Load X from SB -------- */
    printf("Load X from Stack Bus\n");
    bus_drive(SB_IN, X, N);
    en.LOAD_SB_X->value = SIG_1;

    CLK.value = SIG_1;
    regfile_eval(&rf);
    CLK.value = SIG_0;

    en.LOAD_SB_X->value = SIG_0;
    bus_release(SB_IN, N);
    CLK.value = SIG_1;
    regfile_eval(&rf);
    CLK.value = SIG_0;

    dump(&rf);

    /* -------- Enable AC → DB -------- */
    printf("Enable AC onto Data Bus\n");
    en.EN_AC_DB->value = SIG_1;
    CLK.value = SIG_1;
    regfile_eval(&rf);
    CLK.value = SIG_0;
    dump(&rf);

    printf("Enable X onto Stack Bus (keeping AC on DB)\n");
    en.EN_X_SB->value = SIG_1;
        
    CLK.value = SIG_1;
    regfile_eval(&rf);
    CLK.value = SIG_0;

    dump(&rf);
    en.EN_X_SB->value = SIG_0;
    en.EN_AC_DB->value = SIG_0;

    /* -------- RESET PCL to 1 -------- */
    printf("Loading to PLCS to jump counter\n");
    en.EN_PCL_ADL->value = SIG_0;
    bus_drive(ADL_IN, PCLS, N);
    en.LOAD_PCL_PCL->value = SIG_0;
    en.EN_I_PC->value = SIG_0;
    en.LOAD_ADL_PCL->value = SIG_1;

    CLK.value = SIG_1;
    regfile_eval(&rf);
    CLK.value = SIG_0;

    dump(&rf);

    CLK.value = SIG_1;
    regfile_eval(&rf);
    CLK.value = SIG_0;
    dump(&rf);

    CLK.value = SIG_1;
    regfile_eval(&rf);
    CLK.value = SIG_0;
    dump(&rf);

    CLK.value = SIG_1;
    regfile_eval(&rf);
    CLK.value = SIG_0;

    dump(&rf);

    printf("Restart counter\n");
    bus_release(ADL_IN, N);
    en.EN_PCL_ADL->value = SIG_1;
    en.LOAD_PCL_PCL->value = SIG_1; 
    en.EN_I_PC->value = SIG_1;
    en.LOAD_ADL_PCL->value = SIG_0;

    CLK.value = SIG_1;
    regfile_eval(&rf);
    CLK.value = SIG_0;

    dump(&rf);    

    /* -------- Load SP then drive ADL -------- */
    printf("Load SP and drive SB\n");
    bus_drive(SB_IN, SP, N);
    en.LOAD_SB_SP->value = SIG_1;

    CLK.value = SIG_1;
    regfile_eval(&rf);
    CLK.value = SIG_0;

    en.LOAD_SB_SP->value = SIG_0;
    bus_release(SB_IN, N);
    
    CLK.value = SIG_1;
    regfile_eval(&rf);
    CLK.value = SIG_0;

    en.EN_SP_SB->value = SIG_1;
    
    CLK.value = SIG_1;
    regfile_eval(&rf);
    CLK.value = SIG_0;
    
    dump(&rf);

    return 0;
}
