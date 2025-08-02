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

#define INFO_ARRAY_MAX_SIZE 32


// Info text is used to give user any feedback of ..really anything happening.
// from saving a file to glsl errors. It has a setting to be disabled.
struct infotext_t {
    std::string data;
    Color color;
    float timer;
    int   enabled;
};

// Raymarch camera.
struct camera_t {
    Vector3 pos;
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

// Image index for RMSB::res.images
enum ImageIdx : uint16_t {
    EMPTY,
    //...
};

// TODO: Something to debug the values?
//

#define RMSB_MAX_RESOURCE_IMAGES 8


class RMSB {
    public:
        bool running;
        
        Shader           output_shader;  // This shader is for drawing the texture compute shader created.
        uint32_t         compute_shader; // This shader is the user's controlled shader.
        Texture render_texture; // aka Output texture (TODO: Rename this?).

        struct resource_t {
            Texture      images[RMSB_MAX_RESOURCE_IMAGES];
            uint16_t num_images;
        } res;


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

        struct camera_t ray_camera;
        Camera          raster_camera;

        void init(const char* imgui_font_ttf,
                  const char* editor_font_ttf);
        void quit();
        void update();

        // TODO: Add support for reading values back.
        // this is here because of it. (Not implemented yet).
        uint32_t         create_ssbo(int binding_point, size_t size);
     
        Texture create_empty_texture(int width, int height, int format);
        void    delete_texture(Texture* tex);

        void render_3d();
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

        // Call this with NULL, if editing stops.
        void set_position_uniform_ptr(Uniform* ptr);

    private:
        void load_resources();
        void load_resource_img(ImageIdx index, const char* path);

        bool m_user_hold_uniform_pos;
        int  m_user_hold_axis_i;

        Uniform* m_pos_uniform_ptr;

        Vector2 m_mouse_pos;
        struct infotext_t m_infolog[INFO_ARRAY_MAX_SIZE];
        size_t m_infolog_size;

        // When this values is 'true' and RMSB::reload_shader is called,
        // the uniform metadata from the shader file is read.
        // uniform values are saved there so only single file can be needed.
        bool m_first_shader_load;

        void edit_position_uniform();
        void update_camera();
};


#endif
