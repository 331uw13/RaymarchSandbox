
#include <vector>
#include <string_view>
#include <cstring>

#include "preproc.hpp"



struct tag_t {
    size_t index;
    size_t end;
    size_t size;
    const char* pstr; // <-- NOTE: This is _not_ null-terminated.
};


static void find_all_tags(const std::string* shader_code, const char* tag, std::vector<struct tag_t>* out) {
   
    size_t pos = 0;

    while(true) {
        size_t i = shader_code->find(tag, pos);
        if(i == std::string::npos) {
            break;
        }

        size_t next_newln_i = shader_code->find("\n", i, 1);
        size_t next_space_i = shader_code->find(" ", i, 1);
        for(size_t j = next_space_i; j < next_newln_i; j++) {
            if((*shader_code)[j] != ' ') {
                next_space_i = j;
                break;
            }
        }

        if(next_space_i != std::string::npos
        && next_newln_i != std::string::npos
        && next_space_i < next_newln_i) {
            out->push_back(
                    (struct tag_t) {
                        .index = i,
                        .end = next_newln_i,
                        .size = (next_newln_i - next_space_i),
                        .pstr = &((*shader_code)[next_space_i])
                    }
                    );
        }
        pos = i+1;
    }
}


static bool compare(
        const char* str_A, size_t size_A,
        const char* str_B, size_t size_B
){
    if(size_B == 0) {
        size_B = strlen(str_B);
    }

    bool result = false;
    if(size_A != size_B) {
        goto skip;
    }

    for(size_t i = 0; i < size_A; i++) {
        if(str_A[i] != str_B[i]) {
            goto skip;
        }
    }

    result = true;
skip:
    return result;
}


void Preproc::process_glsl(std::string* shader_code, std::string* outdef) {
    std::vector<struct tag_t> include_tags;

    find_all_tags(shader_code, "#include", &include_tags);


    // This can be now easily expanded to include other files for example.
    //

    for(size_t i = 0; i < include_tags.size(); i++) {
        const struct tag_t* tag = &include_tags[i];
    
        if(compare(tag->pstr, tag->size, "RM_VOLUME_MAP", 0)) {
            *outdef += "\n#define VMAP_ENABLED 1\n";
        }
        

        shader_code->erase(tag->index, tag->end - tag->index);
    }
}




