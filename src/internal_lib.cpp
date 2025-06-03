
#include <string>
#include <cstring>
#include "internal_lib.hpp"

#define GLSL_VERSION "#version 330\n"

void InternalLib::create_source() {
    this->source += GLSL_VERSION;
    this->source += "out vec4 out_color;\n";
    this->source += "uniform vec2 screen_size;\n";
    this->source += "uniform float time;\n";
    this->source += "#define PI 3.14159\n";
    this->source += "#define PI2 (PI*2.0)\n";
    this->source += "#define PI_R (PI/180.0)\n";
    this->source += "#define FOV 60.0\n";
    this->source += "#define MAX_RAY_LENGTH 500.0\n";
    this->source += "#define RAY_HIT_DISTANCE 0.001\n";

    const char* RAYHIT_STRUCT = 
        "struct RAYRESULT_T {\n"
        "   int hit;\n"
        "   vec3 color;\n"
        "   float length;\n"  //TODO: Add closest?
        "} RAY_RESULT; \n";

    this->source += RAYHIT_STRUCT;

    this->source += "vec4 map(vec3 p);\n";

    add_func(
            "float SPHERE_SDF(vec3 p, float radius)\n"
            "{\n"
            "    return length(p)-radius;\n"
            "}\n"
            ,
            "Signed distance to sphere.\n"
            );

    add_func(
            "vec3 RAYDIR()\n"
            "{\n"
            "   vec2 rs = screen_size*0.5;\n"
            "   float hf = tan((90.0-FOV*0.5)*(PI_R));\n"
            "   return normalize(vec3(gl_FragCoord.xy-rs, (rs.y*hf)));\n"
            "}\n"
            ,
            "Calculates initial ray direction.\n"
            );

    add_func(
            "void RAYMARCH(vec3 ro, vec3 rd)\n"
            "{\n"
            "   RAY_RESULT.hit = 0;\n"
            "   RAY_RESULT.color = vec3(0, 0, 0);\n"
            "   RAY_RESULT.length = 0.0;\n"
            "   vec3 p = vec3(0, 0, 0);\n"
            "   for(; RAY_RESULT.length < 200.0; ) {\n"
            "      p = ro + rd * RAY_RESULT.length;\n"
            "      vec4 closest = map(p);\n"
            "      if(closest.w < RAY_HIT_DISTANCE) {\n"
            "         RAY_RESULT.hit = 1;\n"
            "         RAY_RESULT.color = closest.rgb;\n"
            "         break;\n"
            "      }\n"
            "      RAY_RESULT.length += closest.w;\n"
            "   }\n"
            "}\n"
            ,
            "ro: Ray origin\n"
            "rd: Ray direction\n"
            "Results of this function can be\n"
            "accessed from 'RAY_RESULT' struct.\n"
            "Notes:\n"
            " * rd must be normalized"
            );
}



const char* InternalLib::get_vertex_src() {
    return 
        GLSL_VERSION
        ""
        ;
}

void InternalLib::add_func(const char* code, const char* description) {
    this->source += code;
    
    struct func_t func = (struct func_t) {
        .code = code,
        .desc = description,
        .name = ""
    };

    size_t code_len = strlen(code);
    for(size_t i = 0; i < code_len; i++) {
        if(code[i] == '\n') {
            break;
        }
        func.name += code[i];
    }

    this->functions.push_back(func);
}

const std::string InternalLib::get_source() {
    return this->source;
}

