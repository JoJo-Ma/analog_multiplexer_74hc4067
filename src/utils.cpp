
#include "utils.h"


std::string shortest_float(float f) {
    if (f == 0.0f) return "0";
    if (f != f) return "NaN";
    // if (f == INFINITY) return "inf";
    // if (f == -INFINITY) return "-inf";

    // Use snprintf with %.9g for float32 round-trip safety
    // This is sufficient for IEEE-754 float32
    char buf[32];
    snprintf(buf, sizeof(buf), "%.9g", f);
    return std::string(buf);
}