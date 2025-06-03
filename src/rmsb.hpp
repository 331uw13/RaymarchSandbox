#ifndef RAYMARCH_SANDBOX_HPP
#define RAYMARCH_SANDBOX_HPP

#include <string>
#include <raylib.h>

#include "rmsb_gui.hpp"
#include "internal_lib.hpp"

#define DEFAULT_WIN_WIDTH 950
#define DEFAULT_WIN_HEIGHT 800


class RMSB {
    public:
        Shader shader;
        bool shader_loaded;

        std::string shader_filepath;

        double time;
        float time_mult;
        bool time_paused;
        
        float file_read_timer;

        RMSBGui gui;

        void init();
        void quit();
        void update();

        void toggle_fullscreen();
        void render_shader();
        void reload_shader();

    private:
        bool m_fullscreen;
        Vector2 m_winsize_nf; // Window size when its not in fullscreen.

};



#endif
