#pragma once
#include "transistor.h"
#include "gates.h"

typedef struct {
    Slot *CLK;
    NOTGate phi1_gate;
    Slot *phi1;
    NOTGate phi2_gate;
    Slot *phi2;
} ClockGen;

void clock_init(ClockGen *ClkGen, Slot *CLK);
void clock_eval(ClockGen *clkGen);