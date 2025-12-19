#include <stdio.h>
#include "../gates.h"

#define S(x) ((x)==SIG_0?"0":(x)==SIG_1?"1":"Z")

int main(void) {
    Slot A_slot, B_slot;
    Slot *A = &A_slot;
    Slot *B = &B_slot;

    NOTGate not;
    NANDGate nand;
    ANDGate andg;
    ORGate org;
    XORGate xorg;

    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            A->value = a ? SIG_1 : SIG_0;
            B->value = b ? SIG_1 : SIG_0;

            not_init(&not, A);
            not_eval(&not);

            nand_init(&nand, A, B);
            nand_eval(&nand);

            and_init(&andg, A, B);
            and_eval(&andg);

            or_init(&org, A, B);
            or_eval(&org);

            xor_init(&xorg, A, B);
            xor_eval(&xorg);

            printf("A=%d B=%d | NOT=%s NAND=%s AND=%s OR=%s XOR=%s\n",
                   a, b,
                   S(not.out.resolved.value),
                   S(nand.out.resolved.value),
                   S(andg.out.resolved.value),
                   S(org.out.resolved.value),
                   S(xorg.out.resolved.value));
        }
    }

    return 0;
}
