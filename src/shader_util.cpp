#include <map>
#include <string_view>
#include <stdio.h>

#include "shader_util.hpp"





bool is_uniform_name_valid(const char* name, size_t name_size) {
    bool is_valid = false;

    if(name_size == 0 || !name) {
        goto not_valid;
    }

    // Name should not start with a number.
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


static std::map<std::string_view, int> g_locations;


static int get_ulocation(Shader* shader, const char* name) {
    auto e = g_locations.find(name);
    int loc = 0;
    if(e != g_locations.end()) {
        loc = g_locations[name];
    }
    else {
        loc = GetShaderLocation(*shader, name);
        if(loc >= 0) {
            g_locations[name] = loc;
        }
    }
    return loc;
}

void shader_util_reset_locations() {
    g_locations.clear();
}

void shader_uniform_float (Shader* shader, const char* name, const float* value) {
    SetShaderValue(*shader, get_ulocation(shader, name), value, SHADER_UNIFORM_FLOAT);
}

void shader_uniform_vec2  (Shader* shader, const char* name, const Vector2* value) {
    SetShaderValue(*shader, get_ulocation(shader, name), value, SHADER_UNIFORM_VEC2);
}

void shader_uniform_vec3  (Shader* shader, const char* name, const Vector3* value) {
    SetShaderValue(*shader, get_ulocation(shader, name), value, SHADER_UNIFORM_VEC3);
}



