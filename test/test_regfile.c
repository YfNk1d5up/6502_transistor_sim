#include <stdio.h>
#include <stdlib.h>
#include "../regfile.h"

int main() {
    int N = 8;
    Slot CLK = {0};
    Slot inputD[8];
    Node outputQ[8];
    RegFile rf;
    RegFileEn en = {0};

    for (int i = 0; i < N; i++) {
        inputD[i].value = SIG_0;
        outputQ[i].resolved.value = SIG_Z;
    }

    // Initialize regfile
    regfile_init(&rf, N, &CLK, inputD, outputQ, &en);

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
        &rf.regA, &rf.regX, &rf.regY, &rf.regSP, &rf.regP, &rf.regPCL, &rf.regPCH
    };

    Slot *regEn[7] = {
        &en.EN_A, &en.EN_X, &en.EN_Y, &en.EN_SP, &en.EN_P, &en.EN_PCL, &en.EN_PCH
    };

    // Write registers
    for (int r = 0; r < 7; r++) {
        printf("Writing to register %d\n", r);
        for (int i = 0; i < N; i++) printf("%d ", testValues[r][i]);
        en.EN_OUT.value = SIG_0;
        regEn[r]->value = SIG_1;
        CLK.value = SIG_1;
        for (int i = 0; i < N; i++) inputD[i].value = testValues[r][i];
        regfile_eval(&rf);
        for (int rp = 0; rp < 7; rp++) {
            printf("\nStored value in register %d :\n", rp);
            for (int i = 0; i < N; i++) printf("%d ", regs[rp]->Q[i].value);
            printf("\n");
        }
        printf("\nOutput of register %d with EN_OUT = 0:\n", r);
        for (int i = 0; i < N; i++) printf("%d ", outputQ[i].resolved.value);
        printf("\n");
        regEn[r]->value = SIG_0;
        CLK.value = SIG_0;
    }

    // input disable
    for (int i = 0; i < N; i++) inputD[i].value = ZValues[i];

    // Read each register individually with EN_OUT = 1
    for (int r = 0; r < 7; r++) {
        en.EN_OUT.value = SIG_1;
        regEn[r]->value = SIG_1;
        CLK.value = SIG_1;
        regfile_eval(&rf);
        for (int rp = 0; rp < 7; rp++) {
            printf("\nStored value in register %d :\n", rp);
            for (int i = 0; i < N; i++) printf("%d ", regs[rp]->Q[i].value);
            printf("\n");
        }
        printf("\nOutput of register %d with EN_OUT = 1:\n", r);
        for (int i = 0; i < N; i++) printf("%d ", rf.outputQ[i].resolved.value);
        printf("\n");
        regEn[r]->value = SIG_0;
        en.EN_OUT.value = SIG_0;
        CLK.value = SIG_0;
    }

    return 0;
}
