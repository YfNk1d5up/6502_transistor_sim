#include "helpers.h"

const char* S(Signal s) {
    switch (s) {
        case SIG_0: return "0";
        case SIG_1: return "1";
        case SIG_Z: return "Z";
    }
    return "?";
}
