#ifndef RAYMARCH_SANDBOX_HPP
#define RAYMARCH_SANDBOX_HPP

#include <string>
#include <raylib.h>

#include "rmsb_gui.hpp"
#include "internal_lib.hpp"
#include "error_log.hpp"

#define DEFAULT_WIN_WIDTH 1200
#define DEFAULT_WIN_HEIGHT 800


#define INFO_ARRAY_MAX_SIZE 32

struct infotext_t {
    std::string data;
    Color color;
    float timer;
    int   enabled;
};

// ----- TODO -------
/*

   - Custom uniforms. Position, Color, Slider value.


*/
class RMSB {
    public:
        Shader shader;
        bool shader_loaded;
        bool show_fps;

        std::string shader_filepath;

        double time;
        float time_mult;
        bool time_paused;
        bool reset_time_on_reload;
        bool show_infolog;
            
        float file_read_timer;
    
        float fov;
        float hit_distance;
        float max_ray_len;

        RMSBGui gui;

        void init();
        void quit();
        void update();

        void toggle_fullscreen();
        void render_shader();
        void reload_shader();

        void loginfo(Color color, const char* text, ...);
        void render_infolog();

    private:
        bool m_first_shader_load;
        bool m_fullscreen;
        Vector2 m_winsize_nf; // Window size when its not in fullscreen.
        
        struct infotext_t m_infolog[INFO_ARRAY_MAX_SIZE];
        size_t m_infolog_size;

};


#endif
