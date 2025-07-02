#ifndef SHADER_UTIL_HPP
#define SHADER_UTIL_HPP

#include <cstddef>
#include <raylib.h>
#include <cstdint>


/*
enum ShaderUniformI {
    U_TIME,
    U_FOV,
    U_HIT_DISTANCE,
    U_MAX_RAY_LENGTH,
    U_SCREEN_SIZE,
    U_CAMERA_INPUT_POS,
    U_CAMERA_INPUT_DIR
};
*/


Shader load_shader_from_mem(const char* vs_code, const char* fs_code);
uint32_t load_compute_shader(const char* code);
void   unload_shader(Shader* shader);

bool is_uniform_name_valid(const char* name, size_t name_size);

void shader_util_reset_locations();
void shader_uniform_float (uint32_t shader, const char* name, const float& value);
void shader_uniform_vec2  (uint32_t shader, const char* name, const Vector2& value);
void shader_uniform_vec3  (uint32_t shader, const char* name, const Vector3& value);
void shader_uniform_vec4  (uint32_t shader, const char* name, const Vector4& value);



#endif
