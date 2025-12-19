#include "gates.h"
#include "register.h"
#include <stdlib.h>

// --- 6502 Register File ---

typedef struct {
    int N; 
    Slot *CLK;      // shared clock
    Slot *inputD;   // shared input bus (8-bit)
    Node *outputQ;  // shared output bus (8-bit)

    // Registers
    NBitRegister regA;   // Accumulator, 8-bit
    NBitRegister regX;   // Index X, 8-bit
    NBitRegister regY;   // Index Y, 8-bit
    NBitRegister regSP;  // Stack Pointer, 8-bit
    NBitRegister regP;   // Processor Status, 8-bit
    NBitRegister regPCL; // Program Counter low byte
    NBitRegister regPCH; // Program Counter high byte

    // Internal storage
    Slot *A;  
    Slot *X;  
    Slot *Y;  
    Slot *SP; 
    Slot *P;  
    Slot *PCL; // 8 bits
    Slot *PCH; // 8 bits

    // AND gates for output enable
    ANDGate *andA;
    ANDGate *andX;
    ANDGate *andY;
    ANDGate *andSP;
    ANDGate *andP;
    ANDGate *andPCL;
    ANDGate *andPCH;

    TriStateGate *tsA;
    TriStateGate *tsX;
    TriStateGate *tsY;
    TriStateGate *tsSP;
    TriStateGate *tsP;
    TriStateGate *tsPCL;
    TriStateGate *tsPCH;

    ANDGate *enA_out;
    ANDGate *enX_out;
    ANDGate *enY_out;
    ANDGate *enSP_out;
    ANDGate *enP_out;
    ANDGate *enPCL_out;
    ANDGate *enPCH_out;
} RegFile;

typedef struct {
    Slot EN_A;
    Slot EN_X;
    Slot EN_Y;
    Slot EN_SP;
    Slot EN_P;
    Slot EN_PCL;
    Slot EN_PCH;
    Slot EN_OUT;
} RegFileEn;

// Initialize the regfile
void regfile_init(RegFile *rf, int N, Slot *CLK, Slot *inputD, Node *outputQ, RegFileEn *en) {
    rf->N = N;
    rf->CLK = CLK;
    rf->inputD = inputD;
    rf->outputQ = outputQ;

    // Allocate storage
    rf->A   = malloc(sizeof(Slot) * N);
    rf->X   = malloc(sizeof(Slot) * N);
    rf->Y   = malloc(sizeof(Slot) * N);
    rf->SP  = malloc(sizeof(Slot) * N);
    rf->P   = malloc(sizeof(Slot) * N);
    rf->PCL = malloc(sizeof(Slot) * N);
    rf->PCH = malloc(sizeof(Slot) * N);

    // Clear storage
    for (int i = 0; i < N; i++) {
        rf->A[i].value   = SIG_0;
        rf->X[i].value   = SIG_0;
        rf->Y[i].value   = SIG_0;
        rf->SP[i].value  = SIG_0;
        rf->P[i].value   = SIG_0;
        rf->PCL[i].value = SIG_0;
        rf->PCH[i].value = SIG_0;
    }

    // Initialize registers
    nreg_init(&rf->regA, N, rf->inputD, rf->A, &en->EN_A);
    nreg_init(&rf->regX, N, rf->inputD, rf->X, &en->EN_X);
    nreg_init(&rf->regY, N, rf->inputD, rf->Y, &en->EN_Y);
    nreg_init(&rf->regSP, N, rf->inputD, rf->SP, &en->EN_SP);
    nreg_init(&rf->regP, N, rf->inputD, rf->P, &en->EN_P);
    nreg_init(&rf->regPCL, N, rf->inputD, rf->PCL, &en->EN_PCL);
    nreg_init(&rf->regPCH, N, rf->inputD, rf->PCH, &en->EN_PCH);

    // Allocate AND gates for output enable
    rf->andA   = malloc(sizeof(ANDGate) * N);
    rf->andX   = malloc(sizeof(ANDGate) * N);
    rf->andY   = malloc(sizeof(ANDGate) * N);
    rf->andSP  = malloc(sizeof(ANDGate) * N);
    rf->andP   = malloc(sizeof(ANDGate) * N);
    rf->andPCL = malloc(sizeof(ANDGate) * N);
    rf->andPCH = malloc(sizeof(ANDGate) * N);

    rf->outputQ = malloc(sizeof(Node) * N);
    for (int i = 0; i < N; i++) {
        rf->outputQ[i].slots = malloc(sizeof(Slot*) * 7); // array of 7 Slot*
        rf->outputQ[i].n_slots = 7;
        for (int j = 0; j < 7; j++) {
            rf->outputQ[i].slots[j] = malloc(sizeof(Slot));  // allocate the actual Slot
            rf->outputQ[i].slots[j]->value = SIG_Z;          // now safe
        }
        rf->outputQ[i].resolved.value = SIG_Z;
    }

    rf->tsA   = malloc(sizeof(TriStateGate) * N);
    rf->tsX   = malloc(sizeof(TriStateGate) * N);
    rf->tsY   = malloc(sizeof(TriStateGate) * N);
    rf->tsSP  = malloc(sizeof(TriStateGate) * N);
    rf->tsP   = malloc(sizeof(TriStateGate) * N);
    rf->tsPCL = malloc(sizeof(TriStateGate) * N);
    rf->tsPCH = malloc(sizeof(TriStateGate) * N);

    rf->enA_out   = malloc(sizeof(ANDGate) * N);
    rf->enX_out   = malloc(sizeof(ANDGate) * N);
    rf->enY_out   = malloc(sizeof(ANDGate) * N);
    rf->enSP_out  = malloc(sizeof(ANDGate) * N);
    rf->enP_out   = malloc(sizeof(ANDGate) * N);
    rf->enPCL_out = malloc(sizeof(ANDGate) * N);
    rf->enPCH_out = malloc(sizeof(ANDGate) * N);

    for (int i = 0; i < N; i++) {
        and_init(&rf->andA[i], &rf->A[i], &en->EN_A);
        and_init(&rf->andX[i], &rf->X[i], &en->EN_X);
        and_init(&rf->andY[i], &rf->Y[i], &en->EN_Y);
        and_init(&rf->andSP[i], &rf->SP[i], &en->EN_SP);
        and_init(&rf->andP[i], &rf->P[i], &en->EN_P);
        and_init(&rf->andPCL[i], &rf->PCL[i], &en->EN_PCL);
        and_init(&rf->andPCH[i], &rf->PCH[i], &en->EN_PCH);
        and_init(&rf->enA_out[i],   &en->EN_A,   &en->EN_OUT);
        and_init(&rf->enX_out[i],   &en->EN_X,   &en->EN_OUT);
        and_init(&rf->enY_out[i],   &en->EN_Y,   &en->EN_OUT);
        and_init(&rf->enSP_out[i],  &en->EN_SP,  &en->EN_OUT);
        and_init(&rf->enP_out[i],   &en->EN_P,   &en->EN_OUT);
        and_init(&rf->enPCL_out[i], &en->EN_PCL, &en->EN_OUT);
        and_init(&rf->enPCH_out[i], &en->EN_PCH, &en->EN_OUT);

        tristate_init(&rf->tsA[i],   &rf->A[i],   &rf->enA_out[i].out.resolved);
        tristate_init(&rf->tsX[i],   &rf->X[i],   &rf->enX_out[i].out.resolved);
        tristate_init(&rf->tsY[i],   &rf->Y[i],   &rf->enY_out[i].out.resolved);
        tristate_init(&rf->tsSP[i],  &rf->SP[i],  &rf->enSP_out[i].out.resolved);
        tristate_init(&rf->tsP[i],   &rf->P[i],   &rf->enP_out[i].out.resolved);
        tristate_init(&rf->tsPCL[i], &rf->PCL[i], &rf->enPCL_out[i].out.resolved);
        tristate_init(&rf->tsPCH[i], &rf->PCH[i], &rf->enPCH_out[i].out.resolved);

        node_add_slot(&rf->outputQ[i], &rf->tsA[i].out.resolved);
        node_add_slot(&rf->outputQ[i], &rf->tsX[i].out.resolved);
        node_add_slot(&rf->outputQ[i], &rf->tsY[i].out.resolved);
        node_add_slot(&rf->outputQ[i], &rf->tsSP[i].out.resolved);
        node_add_slot(&rf->outputQ[i], &rf->tsP[i].out.resolved);
        node_add_slot(&rf->outputQ[i], &rf->tsPCL[i].out.resolved);
        node_add_slot(&rf->outputQ[i], &rf->tsPCH[i].out.resolved);
    }
}

// Evaluate the regfile
void regfile_eval(RegFile *rf) {
    // Evaluate all registers
    nreg_eval(&rf->regA);
    nreg_eval(&rf->regX);
    nreg_eval(&rf->regY);
    nreg_eval(&rf->regSP);
    nreg_eval(&rf->regP);
    nreg_eval(&rf->regPCL);
    nreg_eval(&rf->regPCH);

    // Evaluate AND gates
    for (int i = 0; i < rf->N; i++) {
        and_eval(&rf->andA[i]);
        and_eval(&rf->andX[i]);
        and_eval(&rf->andY[i]);
        and_eval(&rf->andSP[i]);
        and_eval(&rf->andP[i]);
        and_eval(&rf->andPCL[i]);
        and_eval(&rf->andPCH[i]);

        // Evaluate tri-states
        and_eval(&rf->enA_out[i]);
        and_eval(&rf->enX_out[i]);
        and_eval(&rf->enY_out[i]);
        and_eval(&rf->enSP_out[i]);
        and_eval(&rf->enP_out[i]);
        and_eval(&rf->enPCL_out[i]);
        and_eval(&rf->enPCH_out[i]);

        tristate_eval(&rf->tsA[i]);
        tristate_eval(&rf->tsX[i]);
        tristate_eval(&rf->tsY[i]);
        tristate_eval(&rf->tsSP[i]);
        tristate_eval(&rf->tsP[i]);
        tristate_eval(&rf->tsPCL[i]);
        tristate_eval(&rf->tsPCH[i]);

        // Resolve node
        node_resolve(&rf->outputQ[i]);
    }
}
