#ifndef RAYMARCH_SANDBOX_INTERNAL_LIB_HPP
#define RAYMARCH_SANDBOX_INTERNAL_LIB_HPP

#include <string>
#include <list>
#include <unordered_map>
#include <cstdint>

#define CUSTOM_UNIFORMS_TAG_BEGIN "//__tag__UNIFORMS_BEGIN\n"
#define CUSTOM_UNIFORMS_TAG_END   "//__tag__UNIFORMS_END\n"



#define UNIFORM_TYPE_COLOR 0
#define UNIFORM_TYPE_VALUE 1
#define UNIFORM_TYPE_POSITION 2
#define NUM_UNIFORM_TYPES 3

static const int UNIFORM_TYPES[] = {
    UNIFORM_TYPE_COLOR,
    UNIFORM_TYPE_VALUE,
    UNIFORM_TYPE_POSITION
};

static const char* const UNIFORM_TYPES_STR[] = {
    "Color",
    "Value",
    "Position"
};

static const char* const UNIFORM_DATA_TYPES_STR[] = {
    /* Color */    "vec4",
    /* Value */    "float",
    /* Position */ "vec3"
};


struct uniform_t {
    int  type;
    int  location;
    // 0:       Used for floating point value,
    // 0,1,3:   Used for position.
    // 0,1,3,4: Used for color.
    float values[4]; 
    std::string name;
};

struct u8col_t {
    uint8_t red;
    uint8_t grn;
    uint8_t blu;
};

// NOTE: Documents are rendered at 'src/rmsb_gui.cpp' render().
struct document_t {
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
        void add_uniform      (struct uniform_t* u);
        void remove_uniform   (struct uniform_t* u);
        
        const std::string get_source();

        std::list<struct document_t> documents;
        std::list<struct uniform_t> uniforms;

        // Avoid accidental copies.
        InternalLib(InternalLib const&) = delete;
        void operator=(InternalLib const&) = delete;


    private:

        std::string get_uniform_code_line(struct uniform_t* u);

        std::string source;
        InternalLib() {}
};




#endif
