#include "regfile.h"

// Initialize the regfile
void regfile_init(
    RegFile *rf, 
    int N, 
    Slot *CLK, 
    Slot **one, 
    Slot **zero,
    Slot *dummy, 
    Node *dataBus, 
    Node *stackBus, 
    Node *addressLBus, 
    Node *addressHBus, 
    RCL rcl) 
{
    rf->N = N;
    rf->CLK = CLK;
    rf->one = one;
    rf->zero = zero;
    rf->dummy = dummy;
    rf->dataBus = dataBus;
    rf->stackBus = stackBus;
    rf->addressLBus = addressLBus;
    rf->addressHBus = addressHBus;

    // Allocate ports data
    rf->AH06_SB = malloc(sizeof(Slot) * N);
    rf->AH7_SB = malloc(sizeof(Slot) * N);
    rf->AH_ADL = malloc(sizeof(Slot) * N);
    rf->AC_DB   = malloc(sizeof(Slot) * N);
    rf->AC_SB   = malloc(sizeof(Slot) * N);
    rf->X_SB   = malloc(sizeof(Slot) * N);
    rf->Y_SB   = malloc(sizeof(Slot) * N);
    rf->S_SB  = malloc(sizeof(Slot) * N);
    rf->S_ADL = malloc(sizeof(Slot) * N);
    rf->P_DB   = malloc(sizeof(Slot) * N);
    rf->PCL_DB = malloc(sizeof(Slot) * N);
    rf->PCL_ADL = malloc(sizeof(Slot) * N);
    rf->PCH_DB = malloc(sizeof(Slot) * N);
    rf->PCH_ADH = malloc(sizeof(Slot) * N);

    // Clear storage
    for (int i = 0; i < N; i++) {
        rf->AH06_SB[i].value  = SIG_Z;
        rf->AH7_SB[i].value  = SIG_Z;
        rf->AH_ADL[i].value  = SIG_Z;
        rf->AC_DB[i].value  = SIG_Z;
        rf->AC_SB[i].value  = SIG_Z;
        rf->X_SB[i].value   = SIG_Z;
        rf->Y_SB[i].value   = SIG_Z;
        rf->S_SB[i].value  = SIG_Z;
        rf->P_DB[i].value   = SIG_Z;
        rf->PCL_DB[i].value = SIG_Z;
        rf->PCL_ADL[i].value = SIG_Z;
        rf->PCH_DB[i].value = SIG_Z;
        rf->PCH_ADH[i].value = SIG_Z;
    }

    rf->notG = malloc(sizeof(NOTGate) * N); 
    rf->tsNot = malloc(sizeof(TriStateGate) * N);

    rf->notDataBus = malloc(sizeof(Node) * N);
    allocate_node(rf->notDataBus, 1, N);

    rf->dataBusD = malloc(sizeof(Slot*) * N);
    rf->notDataBusD = malloc(sizeof(Slot*) * N);
    rf->stackBusD = malloc(sizeof(Slot*) * N);
    rf->addressLBusD = malloc(sizeof(Slot*) * N);
    rf->addressHBusD = malloc(sizeof(Slot*) * N);
    for (int i = 0; i < N; i++) {
        rf->dataBusD[i] = &rf->dataBus[i].resolved;
        // databus inverter
        not_init(&rf->notG[i], rf->dataBusD[i]);
        //tristate_init(&rf->tsNot[i], &rf->notG[i].out.resolved, rcl.one);
        //node_add_slot(&rf->notDataBus[i], &rf->notG[i].out.resolved);
        rf->notDataBusD[i] = &rf->notG[i].out.resolved;
        rf->stackBusD[i] = &rf->stackBus[i].resolved;
        rf->addressLBusD[i] = &rf->addressLBus[i].resolved;
        rf->addressHBusD[i] = &rf->addressHBus[i].resolved;
    }
    // Initialize registers

    nreg_init(&rf->regA, N, 2, 1, rf->CLK);
    nreg_add_load_port(&rf->regA, 0, rf->zero, rcl.LOAD_0_ADD);
    nreg_add_load_port(&rf->regA, 1, rf->stackBusD, rcl.LOAD_SB_ADD);
    // enable port in ALU

    nreg_init(&rf->regB, N, 3, 1, rf->CLK);
    nreg_add_load_port(&rf->regB, 0, rf->dataBusD, rcl.LOAD_DB_ADD);
    nreg_add_load_port(&rf->regB, 1, rf->notDataBusD, rcl.LOAD_notDB_ADD);
    nreg_add_load_port(&rf->regB, 2, rf->addressLBusD, rcl.LOAD_ADL_ADD);
    // enable port in ALU

    nreg_init(&rf->regAH, N, 1, 3, rf->CLK);
    // load port in ALU
    nreg_add_enable_port(&rf->regAH, 0, rf->AH_ADL, rf->dummy, rcl.EN_ADD_ADL);
    nreg_add_enable_port(&rf->regAH, 1, rf->AH06_SB, rf->dummy, rcl.EN_ADD06_SB);
    nreg_add_enable_port(&rf->regAH, 2, rf->AH7_SB, rf->dummy, rcl.EN_ADD7_SB);

    nreg_init(&rf->regAC, N, 1, 2, rf->CLK);
    nreg_add_load_port(&rf->regAC, 0, rf->stackBusD, rcl.LOAD_SB_AC);
    nreg_add_enable_port(&rf->regAC, 0, rf->AC_DB, rf->dummy, rcl.EN_AC_DB);
    nreg_add_enable_port(&rf->regAC, 1, rf->AC_SB, rf->dummy, rcl.EN_AC_SB);

    nreg_init(&rf->regX, N, 1, 1, rf->CLK);
    nreg_add_load_port(&rf->regX, 0, rf->stackBusD, rcl.LOAD_SB_X);
    nreg_add_enable_port(&rf->regX, 0, rf->X_SB, rf->dummy, rcl.EN_X_SB);

    nreg_init(&rf->regY, N, 1, 1, rf->CLK);
    nreg_add_load_port(&rf->regY, 0, rf->stackBusD, rcl.LOAD_SB_Y);
    nreg_add_enable_port(&rf->regY, 0, rf->Y_SB, rf->dummy, rcl.EN_Y_SB);

    nreg_init(&rf->regSP, N, 1, 2, rf->CLK);
    nreg_add_load_port(&rf->regSP, 0, rf->stackBusD, rcl.LOAD_SB_S);
    nreg_add_enable_port(&rf->regSP, 0, rf->S_SB, rf->dummy, rcl.EN_S_SB);
    nreg_add_enable_port(&rf->regSP, 1, rf->S_ADL, rf->dummy, rcl.EN_S_ADL);

    nreg_init(&rf->regP, N, 6, 1, rf->CLK); // 13 load ports
    nreg_add_load_port(&rf->regP, 0, rf->dataBusD, rcl.LOAD_DB0_P_C);
    nreg_add_load_port(&rf->regP, 1, rf->dataBusD, rcl.LOAD_DB1_P_Z);
    nreg_add_load_port(&rf->regP, 2, rf->dataBusD, rcl.LOAD_DB2_P_I);
    nreg_add_load_port(&rf->regP, 3, rf->dataBusD, rcl.LOAD_DB3_P_D);
    nreg_add_load_port(&rf->regP, 4, rf->dataBusD, rcl.LOAD_DB6_P_V);
    nreg_add_load_port(&rf->regP, 5, rf->dataBusD, rcl.LOAD_DB7_P_N);
    // missing IR5, DBZ, ACR, AVR, I    
    nreg_add_enable_port(&rf->regP, 0, rf->P_DB, rf->dummy, rcl.EN_P_DB);

    nreg_init(&rf->regPCLS, N, 2, 1, rf->CLK);
    nreg_add_load_port(&rf->regPCLS, 0, rf->addressLBusD, rcl.LOAD_ADL_PCL);
    // second one loads from internal PC low bus in PC implementation
 
    nreg_init(&rf->regPCHS, N, 2, 1, rf->CLK);
    nreg_add_load_port(&rf->regPCHS, 0, rf->addressHBusD, rcl.LOAD_ADH_PCH);
    // second one loads from internal PC high bus in PC implementation

    nreg_init(&rf->regPCL, N, 1, 3, rf->CLK);
    // load from internal PC logic in PC implementation
    nreg_add_enable_port(&rf->regPCL, 0, rf->PCL_DB, rf->dummy, rcl.EN_PCL_DB);
    nreg_add_enable_port(&rf->regPCL, 1, rf->PCL_ADL, rf->dummy, rcl.EN_PCL_ADL);
    // third one to internal PC low BUS increment in PC implementation

    nreg_init(&rf->regPCH, N, 1, 3, rf->CLK);
    // load from internal PC logic in PC implementation
    nreg_add_enable_port(&rf->regPCH, 0, rf->PCH_DB, rf->dummy, rcl.EN_PCH_DB);
    nreg_add_enable_port(&rf->regPCH, 1, rf->PCH_ADH, rf->dummy, rcl.EN_PCH_ADH);
    // third one to internal PC low BUS increment in PC implementation

}

void regfile_connect2buses (RegFile *rf) {
    //allocate_node(rf->dataBus, 4, N); // 4 registers

    // Connect output slots to dataBusQ Node (4 registers)
    for (int i = 0; i < rf->N; i++) {
        node_add_slot(&rf->dataBus[i], &rf->AC_DB[i]);
        node_add_slot(&rf->dataBus[i], &rf->P_DB[i]);
        node_add_slot(&rf->dataBus[i], &rf->PCL_DB[i]);
        node_add_slot(&rf->dataBus[i], &rf->PCH_DB[i]);
    }

    //allocate_node(rf->stackBus, 5, N);
    
    // Connect output slots to stackBusQ Node (5 registers)
    for (int i = 0; i < rf->N; i++) {
        if (i == rf->N - 1) 
            node_add_slot(&rf->stackBus[i], &rf->AH7_SB[i]);
        else
            node_add_slot(&rf->stackBus[i], &rf->AH06_SB[i]);
        node_add_slot(&rf->stackBus[i], &rf->AC_SB[i]);
        node_add_slot(&rf->stackBus[i], &rf->X_SB[i]);
        node_add_slot(&rf->stackBus[i], &rf->Y_SB[i]);
        node_add_slot(&rf->stackBus[i], &rf->S_SB[i]);
    }

    //allocate_node(rf->addressLBus, 3, N);

    // Connect output slots to addressLowBusQ Node (3 registers)
    for (int i = 0; i < rf->N; i++) {
        node_add_slot(&rf->addressLBus[i], &rf->AH_ADL[i]);
        node_add_slot(&rf->addressLBus[i], &rf->S_ADL[i]);
        node_add_slot(&rf->addressLBus[i], &rf->PCL_ADL[i]);
    }

    //allocate_node(rf->addressHBus, 1, N);

    // Connect output slots to addressHighBusQ Node (1 register)
    for (int i = 0; i < rf->N; i++) {
        node_add_slot(&rf->addressHBus[i], &rf->PCH_ADH[i]);
    }
}

void resolvenode_regeval(RegFile *rf, NBitRegister *r) {
    // Resolve nodes
    for (int i = 0; i < rf->N; i++) {
        node_resolve(&rf->dataBus[i]);
        not_eval(&rf->notG[i]);
        // is not enough ?
        //tristate_eval(&rf->tsNot[i]);
        //node_resolve(&rf->notDataBus[i]);
        node_resolve(&rf->stackBus[i]);
        node_resolve(&rf->addressLBus[i]);
        node_resolve(&rf->addressHBus[i]);
    }

    nreg_eval(r);

    // Resolve nodes
    for (int i = 0; i < rf->N; i++) {
        node_resolve(&rf->dataBus[i]);
        node_resolve(&rf->notDataBus[i]);
        node_resolve(&rf->stackBus[i]);
        node_resolve(&rf->addressLBus[i]);
        node_resolve(&rf->addressHBus[i]);
    }
}

// Evaluate the regfile
void regfile_eval(RegFile *rf, ProgramCounter *pc, RegALU *alu) {
    
    // Evaluate all registers

    resolvenode_regeval(rf, &rf->regA);
    resolvenode_regeval(rf, &rf->regB);

    alu_eval(alu);

    resolvenode_regeval(rf, &rf->regAH);
    resolvenode_regeval(rf, &rf->regAC);
    resolvenode_regeval(rf, &rf->regX);
    resolvenode_regeval(rf, &rf->regY);
    resolvenode_regeval(rf, &rf->regSP);
    resolvenode_regeval(rf, &rf->regP);
    resolvenode_regeval(rf, &rf->regPCLS);
    resolvenode_regeval(rf, &rf->regPCHS);

    pc_eval(pc);

    resolvenode_regeval(rf, &rf->regPCL);
    resolvenode_regeval(rf, &rf->regPCH);
}