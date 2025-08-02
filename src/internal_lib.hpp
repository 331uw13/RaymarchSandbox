#ifndef RAYMARCH_SANDBOX_INTERNAL_LIB_HPP
#define RAYMARCH_SANDBOX_INTERNAL_LIB_HPP

#include <string>
#include <list>
#include <unordered_map>
#include <cstdint>

#include <raylib.h>


#define CUSTOM_UNIFORMS_TAG_BEGIN "//__tag__UNIFORMS_BEGIN\n"
#define CUSTOM_UNIFORMS_TAG_END   "//__tag__UNIFORMS_END\n"


enum UniformDataType : uint8_t {
    RGBA = 0,
    XYZ,
    SINGLE,
    TEXTURE,

    NUM_TYPES,
    INVALID
};

static const UniformDataType UNIFORM_DATA_TYPES[] = {
    UniformDataType::RGBA,
    UniformDataType::XYZ,
    UniformDataType::SINGLE,
    UniformDataType::TEXTURE
};

static const char* const UNIFORM_DATA_TYPES_STR[] = {
    "RGBA",
    "XYZ",
    "SINGLE",
    "TEXTURE"
};

static const char* const UNIFORM_GLSL_TYPES_STR[] = {
    "vec4",
    "vec3",
    "float",
    "sampler2D"
};


struct Uniform {
    UniformDataType type;
    std::string name;
    int         location; // NOT NEEDED
    

    // 0:       Used for floating point value,
    // 0,1,3:   Used for position.
    // 0,1,3,4: Used for color.
    float values[4];

    Texture   texture;
    bool      has_texture;
    int8_t   texid_for_user;

    Uniform(const std::string& _name, UniformDataType _type) {
        name = _name;
        type = _type;
        has_texture = false;
        texid_for_user = -1;
        location = -1;
        values[0] = 0;
        values[1] = 0;
        values[2] = 0;
        values[3] = 1.0;
    }
};

struct u8col_t {
    uint8_t red;
    uint8_t grn;
    uint8_t blu;
};

// NOTE: Documents are rendered at 'src/rmsb_gui.cpp' render().
struct Document {
    std::string code;
    std::string desc;
    std::string name;
    std::string link; // Link to website for more detailed info. (Not all documents use this)
    size_t num_newlines; // Used for rendering the text field correct size.

    struct u8col_t color;
};

class InternalLib {
    public:
        static InternalLib& get_instance() {
            static InternalLib i;
            return i;
        }
        
        void create_source();
        void add_document     (const char* code, const char* description, struct u8col_t color, const char* link = NULL);
        void add_info         (const char* title, const char* description, struct u8col_t color, const char* link = NULL);
        void add_uniform      (Uniform* u);
        void remove_uniform   (Uniform* u);
        
        const std::string get_source();

        std::list<Document> documents;
        std::list<Uniform> uniforms; // WARNING: Do NOT clear the uniform list directly.
                                     // it will not unload textures if they happens to be loaded.

        // This will correctly delete textures as well
        // if the uniform happens to have one.
        void clear_uniforms();


        // TODO: Rename this funciton, it is too unclear for its purpose.
        void clear();

        // Avoid accidental copies.
        InternalLib(InternalLib const&) = delete;
        void operator=(InternalLib const&) = delete;

        size_t num_lines;

    private:

        std::string get_uniform_code_line(Uniform* u);

        std::string source;
        InternalLib() {}
};




#endif
