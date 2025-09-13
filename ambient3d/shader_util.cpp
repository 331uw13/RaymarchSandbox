#include <map>
#include <cstdio>

#include "raylib.h"

#include "shader_util.hpp"
#include "glsl_preproc.hpp"
#include "external/glad.h"

namespace {
    //         shader id,    uniform name,   uniform location
    static std::map<int, std::map<const char*, int>>
        g_shader_map;

    static constexpr int LOCATION_NOTFOUND = -1;
    static int _find_location(int shader_id, const char* u_name) {

        int location = -1;

        const auto search0 = g_shader_map.find(shader_id);
        if((search0 == g_shader_map.end())
        || (search0->second.find(u_name) == search0->second.end())) {
            location = glGetUniformLocation(shader_id, u_name);
            printf("Found uniform \"%s\" (%i)\n", u_name, location);
            g_shader_map[shader_id].insert(std::make_pair(u_name, location));
        }
        else {
            location = search0->second.find(u_name)->second;
        }

        return location;
    }
};

void AM::init_instanced_shader(Shader* shader) {
    shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
}

// TODO: Implement something when the value has not changed, it dont need to be updated.
 
void AM::set_uniform_int(int shader_id, const char* uniform_name, int value) {
    glUseProgram(shader_id);
    glUniform1i(_find_location(shader_id, uniform_name), value);
}   

void AM::set_uniform_float(int shader_id, const char* uniform_name, float value) {
    glUseProgram(shader_id);
    glUniform1f(_find_location(shader_id, uniform_name), value);
}

void AM::set_uniform_vec2(int shader_id, const char* uniform_name, const Vector2& value) {
    glUseProgram(shader_id);
    glUniform2f(_find_location(shader_id, uniform_name), value.x, value.y);
}

void AM::set_uniform_vec3(int shader_id, const char* uniform_name, const Vector3& value) {
    glUseProgram(shader_id);
    glUniform3f(_find_location(shader_id, uniform_name), value.x, value.y, value.z);
}
 
void AM::set_uniform_vec4(int shader_id, const char* uniform_name, const Vector4& value) {
    glUseProgram(shader_id);
    glUniform4f(_find_location(shader_id, uniform_name), value.x, value.y, value.z, value.w);
}
    

void AM::set_uniform_matrix(int shader_id, const char* uniform_name, const Matrix& value) {
    glUseProgram(shader_id);
    glUniformMatrix3fv(_find_location(shader_id, uniform_name), 1, false, &value.m0);
}

    
void AM::set_uniform_sampler(int shader_id, const char* uniform_name, const Texture2D& texture, int slot) {
    glUseProgram(shader_id);
    int location = _find_location(shader_id, uniform_name);
    if(location) {
        glUniform1i(location, slot);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texture.id);
    }
}


