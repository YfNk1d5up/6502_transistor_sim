#include "clock.h"

void clock_init(ClockGen *clkGen, Slot *CLK) {
    not_init(&clkGen->phi1_gate, CLK);
    clkGen->phi1 = &clkGen->phi1_gate.out.resolved;
    not_init(&clkGen->phi2_gate, &clkGen->phi1_gate.out.resolved);
    clkGen->phi2 = &clkGen->phi2_gate.out.resolved;
}

void clock_eval(ClockGen *clkGen) {
    not_eval(&clkGen->phi1_gate);
    not_eval(&clkGen->phi2_gate);    
}