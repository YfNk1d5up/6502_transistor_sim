#include <stdio.h>
#include <stdlib.h>
#include "../regfile.h"

typedef struct {
    Slot LOAD_AC;
    Slot LOAD_X;
    Slot LOAD_Y;
    Slot LOAD_SP;
    Slot LOAD_P;
    Slot LOAD_PC;
    Slot LOAD_PCL;
    Slot LOAD_PCH;
    Slot EN_AC;
    Slot EN_X;
    Slot EN_Y;
    Slot EN_SP;
    Slot EN_P;
    PCCtl PCCTL;
    Slot EN_PCL;
    Slot EN_PCH;
} En;

int main() {
    int N = 8;
    Slot CLK = {0};
    Slot *inputD[8];
    Slot *outputQ[8];
    RegFile rf;
    RegFileEn rf_en;
    En en;

    en.LOAD_AC.value = SIG_0;
    en.LOAD_X.value = SIG_0;
    en.LOAD_Y.value = SIG_0;
    en.LOAD_SP.value = SIG_0;
    en.LOAD_P.value = SIG_0;
    en.LOAD_PC.value = SIG_0;
    en.LOAD_PCL.value = SIG_0;
    en.LOAD_PCH.value = SIG_0;
    en.EN_AC.value = SIG_0;
    en.EN_X.value = SIG_0;
    en.EN_Y.value = SIG_0;
    en.EN_SP.value = SIG_0;
    en.EN_P.value = SIG_0;
    en.PCCTL.LOAD_PCH = malloc(sizeof(Slot));
    en.PCCTL.LOAD_PCL = malloc(sizeof(Slot));
    en.PCCTL.EN_PCH = malloc(sizeof(Slot));
    en.PCCTL.EN_PCL = malloc(sizeof(Slot));
    en.PCCTL.EN_CNT = malloc(sizeof(Slot));
    en.PCCTL.EN_CNT->value = SIG_1;
    en.EN_PCL.value = SIG_0;
    en.EN_PCH.value = SIG_0;

    rf_en.LOAD_AC = &en.LOAD_AC;
    rf_en.LOAD_X = &en.LOAD_X;
    rf_en.LOAD_Y = &en.LOAD_Y;
    rf_en.LOAD_SP = &en.LOAD_SP;
    rf_en.LOAD_P = &en.LOAD_P;
    rf_en.LOAD_PCL = &en.LOAD_PCL;
    rf_en.LOAD_PCH = &en.LOAD_PCH;
    rf_en.EN_AC = &en.EN_AC;
    rf_en.EN_X = &en.EN_X;
    rf_en.EN_Y = &en.EN_Y;
    rf_en.EN_SP = &en.EN_SP;
    rf_en.EN_P = &en.EN_P;
    rf_en.PCCTL = en.PCCTL;
    rf_en.EN_PCL = &en.EN_PCL;
    rf_en.EN_PCH = &en.EN_PCH;

    for (int i = 0; i < N; i++) {
        inputD[i] = malloc(sizeof(Slot));
        inputD[i]->value = SIG_0;
    }

    // Initialize regfile
    regfile_init(&rf, N, &CLK, inputD, &rf_en);

    for (int i = 0; i < N; i++) {
        outputQ[i] = malloc(sizeof(Slot));
        outputQ[i] = &rf.outputQ[i].resolved;
    }


    int testValues[7][8] = {
        {1,0,1,0,1,0,1,0}, // A
        {0,1,0,1,0,1,0,1}, // X
        {1,1,0,0,1,1,0,0}, // Y
        {0,0,1,1,0,0,1,1}, // SP
        {1,0,0,1,1,0,0,1}, // P
        {0,1,1,0,0,1,1,0}, // PCL
        {1,0,0,1,0,1,0,1}, // PCH
    };
    int ZValues[8] = {SIG_Z,SIG_Z,SIG_Z,SIG_Z,SIG_Z,SIG_Z,SIG_Z,SIG_Z};

    NBitRegister *regs[7] = {
        &rf.regAC, &rf.regX, &rf.regY, &rf.regSP, &rf.regP, &rf.regPCL, &rf.regPCH
    };

    Slot *regLoad[7] = {
        &en.LOAD_AC, &en.LOAD_X, &en.LOAD_Y, &en.LOAD_SP, &en.LOAD_P, &en.LOAD_PCL, &en.LOAD_PCH
    };
    Slot *regEn[7] = {
        &en.EN_AC, &en.EN_X, &en.EN_Y, &en.EN_SP, &en.EN_P, &en.EN_PCL, &en.EN_PCH
    };

    // Write registers
    for (int r = 0; r < 5; r++) { // stop at 5 so PCH and PCL are not loaded
        printf("PCCTL.EN_CNT = %d\n", rf_en.PCCTL.EN_CNT->value);

        printf("Writing to register %d\n", r);
        for (int i = 0; i < N; i++) printf("%d ", testValues[r][i]);
        regLoad[r]->value = SIG_1;
        CLK.value = SIG_1;
        for (int i = 0; i < N; i++) inputD[i]->value = testValues[r][i];
        regfile_eval(&rf);
        for (int rp = 0; rp < 7; rp++) {
            printf("\nStored value in register %d :\n", rp);
            for (int i = 0; i < N; i++) printf("%d ", regs[rp]->Q[i].value);
            printf("\n");
        }
        printf("\nOutput of register %d with EN_OUT = 0:\n", r);
        for (int i = 0; i < N; i++) printf("%d ", outputQ[i]->value);
        printf("\n");
        regLoad[r]->value = SIG_0;
        regEn[r]->value = SIG_0;
        CLK.value = SIG_0;
        regfile_eval(&rf);
    }

    // input disable
    for (int i = 0; i < N; i++) inputD[i]->value = ZValues[i];

    // Read each register individually with EN_OUT = 1
    for (int r = 0; r < 7; r++) {
        printf("Reading from register %d\n", r);
        for (int i = 0; i < N; i++) printf("%d ", testValues[r][i]);
        regLoad[r]->value = SIG_0;
        regEn[r]->value = SIG_1;
        CLK.value = SIG_1;
        for (int i = 0; i < N; i++) inputD[i]->value = testValues[r][i];
        regfile_eval(&rf);
        printf("\n");
        for (int rp = 0; rp < 7; rp++) {
            printf("Stored value in register %d :\n", rp);
            for (int i = 0; i < N; i++) printf("%d ", regs[rp]->Q[i].value);
            printf("\n");
        }
        printf("\nOutput of register %d with EN_OUT = 0:\n", r);
        for (int i = 0; i < N; i++) printf("%d ", outputQ[i]->value);
        printf("\n");
        regLoad[r]->value = SIG_0;
        regEn[r]->value = SIG_0;
        CLK.value = SIG_0;
        regfile_eval(&rf);
    }

    return 0;
}
