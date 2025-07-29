#ifndef RAYMARCH_SANDBOX_HPP
#define RAYMARCH_SANDBOX_HPP

#include <string>
#include <raylib.h>

#include "rmsb_gui.hpp"
#include "internal_lib.hpp"
#include "error_log.hpp"
#include "editor.hpp"
#include "filebrowser.hpp"


#define GLSL_VERSION "#version 430\n"

#define DEFAULT_WIN_WIDTH 1200
#define DEFAULT_WIN_HEIGHT 800

#define STARTUP_CMD_BEGIN_TAG "@startup_cmd"
#define STARTUP_CMD_END_TAG   "@end"

#define INFO_ARRAY_MAX_SIZE 32


// Info text is used to give user any feedback of ..really anything happening.
// from saving a file to glsl errors. It has a setting to be disabled.
struct infotext_t {
    std::string data;
    Color color;
    float timer;
    int   enabled;
};

struct camera_t {
    Vector3 pos;
    Vector3 dir;
    float yaw;
    float pitch;
    float sensetivity;
    float move_speed;
};

// Different mode allows to have same keybind but different action
// depending on what mode is enabled.
enum Mode {
    VIEW_MODE,
    EDIT_MODE
};

// TODO: Rename ??????
struct texture_t {
    uint32_t id;
    int format;
    int width;
    int height;
};

// TODO: Something to debug the values?
//

class RMSB {
    public:
        bool running;
        
        Shader           output_shader;  // This shader is for drawing the texture compute shader created.
        uint32_t         compute_shader; // This shader is the user's controlled shader.
        struct texture_t render_texture; // aka Output texture (TODO: Rename this?).


        // TODO: Add settings struct to make this more organized.

        std::string shader_filepath;

        double time;
        float time_mult;
        float auto_reload_delay;
        bool auto_reload;
        bool time_paused;
        bool reset_time_on_reload;
        bool show_infolog;
        bool allow_camera_input;
        int fps_limit;
        bool show_fps;

        float file_read_timer;
    
        float fov;
        float hit_distance;
        float max_ray_len;
        float translucent_step_size;
        
        // Ambient occlusion settings.
        int   ao_num_samples;
        float ao_step_size;
        float ao_falloff;
            
        int monitor_width;
        int monitor_height;

        RMSBGui      gui;

        struct camera_t camera;

        void init();
        void quit();
        void update();

        // TODO: Add support for reading values back.
        // this is here because of it. (Not implemented yet).
        uint32_t         create_ssbo(int binding_point, size_t size);
        
        struct texture_t create_empty_texture(int width, int height, int format);
        void             delete_texture(struct texture_t* tex);

        void render_shader();
        
        void reload_shader();
        void reload_lib();
        
        // Reload shader,
        // Reload internal lib,
        // Clear custom uniform inputs.
        // Clear undo stack.
        // Set m_first_shader_loaded = true
        // Reset camera.
        void reload_state();


        void loginfo(Color color, const char* text, ...);
        void render_infolog();

        enum Mode mode;

        int input_key; // See 'src/input.cpp'


    private:

        Vector2 m_mouse_pos;
        struct infotext_t m_infolog[INFO_ARRAY_MAX_SIZE];
        size_t m_infolog_size;

        // When this values is 'true'
        // and RMSB::reload_shader is called,
        // the custom uniform settings(aka startup cmd) is parsed
        // from the shader file.
        bool m_first_shader_load;

        // On first load need to add the uniforms that are in.
        // @startup_command .... @end  region.
        // TODO: Move these maybe somewhere else ???
        void run_shader_startup_cmd(const std::string* shader_code);
        void get_cmdline_value(const std::string& code_line, float values[4]);
        void process_shader_startup_cmd_line(const std::string& code_line);
        
    
        // @startup_command  and @end are not valid glsl code.
        // they must be removed after reading.
        void remove_startup_cmd_blocks(std::string* shader_code);

        void update_camera();
};


#endif
