#include <stdio.h>
#include <cstring>

#include "util.hpp"
#include "rmsb.hpp"

int64_t iclamp64(int64_t i, int64_t min, int64_t max) {
    if(i < min) { i = min; }
    else
    if(i > max) { i = max; }

    return i;
}

/*
void save_uniform_values(std::string& shader_code, const struct uniform_t* uniform) {
    size_t begin_index = shader_code.find(STARTUP_CMD_BEGIN_TAG);
    size_t end_index = shader_code.find(STARTUP_CMD_END_TAG);

    
    if((begin_index == std::string::npos) || (end_index == std::string::npos)) {
        printf("'%s': No valid startup command region found.\n", __func__);
        return;
    }

    printf("Save: %s: %f, %f, %f, %f\n",
            name,
            values[0],
            values[1],
            values[2],
            values[3]
            );
   
    size_t name_size = strlen(name);
    size_t name_index = shader_code.find(name, begin_index);
    if(name_index == std::string::npos) {
        // New uniform was added from gui.
        // It doesnt have name in the shader code yet so add it.

        std::string new_value_str = TextFormat(
                "ADD POSITION %s (%0.3f, %0.3f, %0.3f, %0.3f);",
                name,
                values[0], 
                values[1], 
                values[2], 
                values[3] 
                );
        shader_code.insert(begin_index+strlen(STARTUP_CMD_BEGIN_TAG)+1, new_value_str);

       return;
    }

    name_index += name_size;

    size_t name_end = shader_code.find(";", name_index);
    if(name_end == std::string::npos) {
        fprintf(stderr, "'%s': ';' expected but not found for \"%s\"\n",
                __func__, name);
        return;
    }


    shader_code.erase(name_index, name_end - name_index);

    const std::string new_str_value = TextFormat(
            " (%0.3f, %0.3f, %0.3f, %0.3f)",
            values[0], values[1], values[2], values[3]);


    shader_code.insert(name_index, new_str_value);
}*/

