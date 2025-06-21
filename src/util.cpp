#include "util.hpp"



int64_t iclamp64(int64_t i, int64_t min, int64_t max) {
    return (i < min) ? min : (i > max) ? max : i;
}



