#include <map>
#include <string_view>
#include <stdio.h>
#include <cstring>

#include "libs/glad.h"

#include "shader_util.hpp"
#include "error_log.hpp"

#include <rlgl.h>


unsigned int compile_shader(const char* shader_code, int shader_type) {
    unsigned int shader = 0;
    shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_code, NULL);

    int shader_ok = 0;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);

    if(!shader_ok) { 
        int log_max_len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_max_len);

        if(log_max_len > 0) {
            int log_len = 0;
            char* log = (char*)malloc(log_max_len);
            if(!log) {
                fprintf(stderr, "'%s': Failed to allocate memory for log.\n",
                        __func__);
       
                glDeleteShader(shader);
                shader = 0;
                goto error;
            }
     
            glGetShaderInfoLog(shader, log_max_len, &log_len, log);
    
            //printf("%s: %s\n", __func__, log);
            ErrorLog::get_instance().add(log);
            free(log);
        }

        glDeleteShader(shader);
        shader = 0;
    }

error:
    return shader;
}


// Yes, raylib has its own LoadShaderFromMemory function
// but it may leak memory if the shader compiling fail.
// had fix it and copy these DEFAULT_SHADER_ATTRIB... stuff here.


// Default shader vertex attribute names to set location points
// Bound by default to shader location:RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION
#define RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION     "vertexPosition"
// Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD     "vertexTexCoord"
// Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL
#define RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL       "vertexNormal"
// Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR
#define RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR        "vertexColor"
// Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT      "vertexTangent"
// Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2    "vertexTexCoord2"
// Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_BONEIDS
#define RL_DEFAULT_SHADER_ATTRIB_NAME_BONEIDS      "vertexBoneIds"
// Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_BONEWEIGHTS
#define RL_DEFAULT_SHADER_ATTRIB_NAME_BONEWEIGHTS  "vertexBoneWeights"

#define RL_DEFAULT_SHADER_UNIFORM_NAME_MVP         "mvp"               // model-view-projection matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW        "matView"           // view matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION  "matProjection"     // projection matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL       "matModel"          // model matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL      "matNormal"         // normal matrix (transpose(inverse(matModelView))
#define RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR       "colDiffuse"        // color diffuse (base tint color, multiplied by texture color)
#define RL_DEFAULT_SHADER_UNIFORM_NAME_BONE_MATRICES  "boneMatrices"   // bone matrices
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0  "texture0"          // texture0 (texture slot active 0)
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1  "texture1"          // texture1 (texture slot active 1)
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2  "texture2"          // texture2 (texture slot active 2)

static void set_shader_attrib_locs(Shader* shader) {
    shader->locs = (int*)malloc(RL_MAX_SHADER_LOCATIONS * sizeof(int));
    memset(shader->locs, -1, RL_MAX_SHADER_LOCATIONS * sizeof(int));

    shader->locs[SHADER_LOC_VERTEX_POSITION] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION);
    shader->locs[SHADER_LOC_VERTEX_TEXCOORD01] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD);
    shader->locs[SHADER_LOC_VERTEX_TEXCOORD02] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2);
    shader->locs[SHADER_LOC_VERTEX_NORMAL] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL);
    shader->locs[SHADER_LOC_VERTEX_TANGENT] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT);
    shader->locs[SHADER_LOC_VERTEX_COLOR] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR);
    shader->locs[SHADER_LOC_VERTEX_BONEIDS] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_BONEIDS);
    shader->locs[SHADER_LOC_VERTEX_BONEWEIGHTS] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_BONEWEIGHTS);
    //shader->locs[SHADER_LOC_VERTEX_INSTANCE_TX] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_INSTANCE_TX);

    // Get handles to GLSL uniform locations (vertex shader)
    shader->locs[SHADER_LOC_MATRIX_MVP] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_UNIFORM_NAME_MVP);
    shader->locs[SHADER_LOC_MATRIX_VIEW] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW);
    shader->locs[SHADER_LOC_MATRIX_PROJECTION] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION);
    shader->locs[SHADER_LOC_MATRIX_MODEL] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL);
    shader->locs[SHADER_LOC_MATRIX_NORMAL] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL);
    shader->locs[SHADER_LOC_BONE_MATRICES] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_UNIFORM_NAME_BONE_MATRICES);

    // Get handles to GLSL uniform locations (fragment shader)
    shader->locs[SHADER_LOC_COLOR_DIFFUSE] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR);
    shader->locs[SHADER_LOC_MAP_DIFFUSE] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0);  // SHADER_LOC_MAP_ALBEDO
    shader->locs[SHADER_LOC_MAP_SPECULAR] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1); // SHADER_LOC_MAP_METALNESS
    shader->locs[SHADER_LOC_MAP_NORMAL] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2);

}
static void bind_program_attrib_locs(unsigned int program) {
    glBindAttribLocation(program, RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION);
    glBindAttribLocation(program, RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD);
    glBindAttribLocation(program, RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL, RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL);
    glBindAttribLocation(program, RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR, RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR);
    glBindAttribLocation(program, RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT, RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT);
    glBindAttribLocation(program, RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2);
}

Shader load_shader_from_mem(const char* vs_code, const char* fs_code) {
    Shader shader = (Shader){ 0, NULL };
    unsigned int program = 0;
    int program_ok = 0;

    unsigned int vertex_id = compile_shader(vs_code, GL_VERTEX_SHADER);
    unsigned int fragment_id = compile_shader(fs_code, GL_FRAGMENT_SHADER);

    ErrorLog& error_log = ErrorLog::get_instance();

    if(vertex_id == 0) {
        error_log.add("Vertex shader failed.");
        goto error;
    }

    if(fragment_id == 0) {
        error_log.add("Fragment shader failed.");
        goto error;
    }


    program = glCreateProgram();

    glAttachShader(program, vertex_id);
    glAttachShader(program, fragment_id);

    bind_program_attrib_locs(program);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if(!program_ok) {
        int log_max_len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_max_len);

        if(log_max_len > 0) {
            int log_len = 0;
            char* log = (char*)malloc(log_max_len);
            if(!log) {
                fprintf(stderr, "'%s': Failed to allocate memory for log.\n",
                        __func__);

                glDeleteProgram(program);
                goto error;
            }
     
            glGetProgramInfoLog(program, log_max_len, &log_len, log);
            
            error_log.add(log);
            free(log);
        }

        glDeleteProgram(program);
        error_log.add("Shader program linking failed.");
        goto error;
    }

    glDetachShader(program, vertex_id);
    glDetachShader(program, fragment_id);

    glDeleteShader(vertex_id);
    glDeleteShader(fragment_id);

    shader.id = program;

    set_shader_attrib_locs(&shader);

error:
    return shader;
}

void unload_shader(Shader* shader) {
    if(shader->id > 0) {
        glDeleteProgram(shader->id);
        shader->id = 0;
    }

    if(shader->locs) {
        free(shader->locs);
    }

}

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



