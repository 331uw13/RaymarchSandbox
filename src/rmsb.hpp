#ifndef RAYMARCH_SANDBOX_HPP
#define RAYMARCH_SANDBOX_HPP

#include <string>
#include <raylib.h>

#include "rmsb_gui.hpp"
#include "internal_lib.hpp"
#include "error_log.hpp"
#include "editor.hpp"

#define GLSL_VERSION "#version 430\n"

#define DEFAULT_WIN_WIDTH 1200
#define DEFAULT_WIN_HEIGHT 800

#define STARTUP_CMD_BEGIN_TAG "@startup_cmd"
#define STARTUP_CMD_END_TAG   "@end"

#define INFO_ARRAY_MAX_SIZE 32

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

enum Mode {
    VIEW_MODE,
    EDIT_MODE
};

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
        
        Shader output_shader;
        uint32_t compute_shader;
        struct texture_t render_texture;
        //uint32_t render_texture;

        bool show_fps;

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

        float file_read_timer;
    
        float fov;
        float hit_distance;
        float max_ray_len;

        int monitor_width;
        int monitor_height;

        RMSBGui gui;
        struct camera_t camera;

        void init();
        void quit();
        void update();

        uint32_t create_ssbo(int binding_point, size_t size);
        struct texture_t create_empty_texture(int width, int height, int format);

        void render_shader();
        void reload_shader();

        void reload_lib();

        void loginfo(Color color, const char* text, ...);
        void render_infolog();

        enum Mode mode;

        int input_key; // See 'src/input.cpp'


    private:

        bool m_first_shader_load;
        Vector2 m_screen_size_prev; // Changed by 'set_fullscreen()' function.
        Vector2 m_mouse_pos;
        struct infotext_t m_infolog[INFO_ARRAY_MAX_SIZE];
        size_t m_infolog_size;

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
