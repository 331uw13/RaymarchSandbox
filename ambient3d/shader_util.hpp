#ifndef AMBIENT3D_SHADER_UTIL_HPP
#define AMBIENT3D_SHADER_UTIL_HPP


namespace AM {

    void init_instanced_shader(Shader* shader);

    void set_uniform_int    (int shader_id, const char* uniform_name, int value);
    void set_uniform_float  (int shader_id, const char* uniform_name, float value);
    void set_uniform_vec2   (int shader_id, const char* uniform_name, const Vector2& value);
    void set_uniform_vec3   (int shader_id, const char* uniform_name, const Vector3& value);
    void set_uniform_vec4   (int shader_id, const char* uniform_name, const Vector4& value);
    void set_uniform_matrix (int shader_id, const char* uniform_name, const Matrix&  value);
    void set_uniform_sampler(int shader_id, const char* uniform_name, const Texture2D& texture, int slot);
};



#endif
