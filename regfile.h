#include "gates.h"
#include "register.h"
#include "pc.h"
#include <stdlib.h>

// --- 6502 Register File ---

typedef struct {
    int N; 
    Slot *CLK;      // shared clock
    Slot **inputD;   // shared input bus (8-bit)
    Node *outputQ;  // shared output bus (8-bit)

    // Registers
    NBitRegister regAC;   // Accumulator, 8-bit
    NBitRegister regX;   // Index X, 8-bit
    NBitRegister regY;   // Index Y, 8-bit
    NBitRegister regSP;  // Stack Pointer, 8-bit
    NBitRegister regP;   // Processor Status, 8-bit
    NBitRegister regPCL; // Program Counter low byte
    NBitRegister regPCH; // Program Counter high byte

    // Registers Outputs
    Slot *AC;  
    Slot *X;  
    Slot *Y;  
    Slot *SP; 
    Slot *P;  
    Slot *PCL; // 8 bits
    Slot *PCH; // 8 bits


    // AND gates for output enable
    ANDGate *andAC;
    ANDGate *andX;
    ANDGate *andY;
    ANDGate *andSP;
    ANDGate *andP;
    ANDGate *andPCL;
    ANDGate *andPCH;

    ProgramCounter pc;
} RegFile;

typedef struct {
    Slot *LOAD_AC;
    Slot *LOAD_X;
    Slot *LOAD_Y;
    Slot *LOAD_SP;
    Slot *LOAD_P;
    Slot *LOAD_PC;
    Slot *LOAD_PCL;
    Slot *LOAD_PCH;
    Slot *EN_AC;
    Slot *EN_X;
    Slot *EN_Y;
    Slot *EN_SP;
    Slot *EN_P;
    PCCtl PCCTL;
    Slot *EN_PCL;
    Slot *EN_PCH;
} RegFileEn;

// Initialize the regfile
void regfile_init(RegFile *rf, int N, Slot *CLK, Slot **inputD, RegFileEn *en) {
    rf->N = N;
    rf->CLK = CLK;
    rf->inputD = inputD;

    // Allocate storage
    rf->AC   = malloc(sizeof(Slot) * N);
    rf->X   = malloc(sizeof(Slot) * N);
    rf->Y   = malloc(sizeof(Slot) * N);
    rf->SP  = malloc(sizeof(Slot) * N);
    rf->P   = malloc(sizeof(Slot) * N);
    rf->PCL = malloc(sizeof(Slot) * N);
    rf->PCH = malloc(sizeof(Slot) * N);

    // Clear storage
    for (int i = 0; i < N; i++) {
        rf->AC[i].value  = SIG_0;
        rf->X[i].value   = SIG_0;
        rf->Y[i].value   = SIG_0;
        rf->SP[i].value  = SIG_0;
        rf->P[i].value   = SIG_0;
        rf->PCL[i].value = SIG_0;
        rf->PCH[i].value = SIG_0;
    }

    // Initialize registers
    nreg_init(&rf->regAC, N, rf->inputD, rf->AC, en->LOAD_AC, en->EN_AC, rf->CLK);
    nreg_init(&rf->regX, N, rf->inputD, rf->X, en->LOAD_X, en->EN_X, rf->CLK);
    nreg_init(&rf->regY, N, rf->inputD, rf->Y, en->LOAD_Y, en->EN_Y, rf->CLK);
    nreg_init(&rf->regSP, N, rf->inputD, rf->SP, en->LOAD_SP, en->EN_SP, rf->CLK);
    nreg_init(&rf->regP, N, rf->inputD, rf->P, en->LOAD_P, en->EN_P, rf->CLK);

    en->PCCTL.EN_PCL = en->EN_PCL;
    en->PCCTL.EN_PCH = en->EN_PCH;
    pc_init(&rf->pc, N, CLK, en->PCCTL, &rf->regPCL, &rf->regPCH);
    //nreg_init(&rf->regPCL, N, rf->inputD, rf->PCL, en->EN_PCL);
    //nreg_init(&rf->regPCH, N, rf->inputD, rf->PCH, en->EN_PCH);


    rf->outputQ = malloc(sizeof(Node) * N);
    for (int i = 0; i < N; i++) {
        rf->outputQ[i].slots = malloc(sizeof(Slot*) * 7); // array of 7 Slot* ( for the 7 registers)
        rf->outputQ[i].n_slots = 7;
        for (int j = 0; j < 7; j++) {
            rf->outputQ[i].slots[j] = malloc(sizeof(Slot));  // allocate the actual Slot
            rf->outputQ[i].slots[j]->value = SIG_Z;          // now safe
        }
        rf->outputQ[i].resolved.value = SIG_Z;
    }

    for (int i = 0; i < N; i++) {
        node_add_slot(&rf->outputQ[i], &rf->regAC.Q[i]);
        node_add_slot(&rf->outputQ[i], &rf->regX.Q[i]);
        node_add_slot(&rf->outputQ[i], &rf->regY.Q[i]);
        node_add_slot(&rf->outputQ[i], &rf->regSP.Q[i]);
        node_add_slot(&rf->outputQ[i], &rf->regP.Q[i]);
        node_add_slot(&rf->outputQ[i], &rf->regPCL.Q[i]);
        node_add_slot(&rf->outputQ[i], &rf->regPCH.Q[i]);
    }
}

// Evaluate the regfile
void regfile_eval(RegFile *rf) {
    // Evaluate all registers
    nreg_eval(&rf->regAC);
    nreg_eval(&rf->regX);
    nreg_eval(&rf->regY);
    nreg_eval(&rf->regSP);
    nreg_eval(&rf->regP);
    nreg_eval(&rf->regPCL);
    nreg_eval(&rf->regPCH);

    // Evaluate AND gates
    for (int i = 0; i < rf->N; i++) {
        // Resolve node
        node_resolve(&rf->outputQ[i]);
    }
}