#include "util.hpp"








bool is_uniform_name_valid(const char* name, size_t name_size) {
    bool is_valid = false;

    if(name_size == 0 || !name) {
        goto not_valid;
    }

    /*
    // Name should not start with a number.
    if(name[0] >= 0x30 || name[0] <= 0x39) {
        goto not_valid;
    }
    */
    if((name[0] >= '0') && (name[0] <= '9')) {
        goto not_valid;
    }

    for(size_t i = 0; i < name_size; i++) {
        char c = name[i];
        if(c == '_') {
            continue;
        }

        // Dont allow any special characters.
        // Some of them are located between non-special characters 
        // so they have to be checked too.
        if((c < 0x30) || (c > 0x7A)) {
            goto not_valid;
        }
        if((c >= 0x5C) && (c <= 0x60)) {
            goto not_valid;
        }
        if((c >= 0x3A) && (c <= 0x40)) {
        }
    }

    is_valid = true;

not_valid:
    return is_valid;
}



