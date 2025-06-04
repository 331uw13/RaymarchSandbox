#include <raylib.h>
#include <stdio.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "rmsb.hpp"



void RMSB::init() {
    printf("%s\n", __func__);

    InitWindow(
        DEFAULT_WIN_WIDTH,
        DEFAULT_WIN_HEIGHT,
        "Raymarch Sandbox"
    );
    SetWindowMinSize(200, 200);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(300);


    m_winsize_nf = (Vector2){ DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT };
    m_fullscreen = false;
    this->time_paused = false;
    this->time_mult = 1.0f;
    this->time = 0.0;
    this->file_read_timer = 0.0f;
    this->shader_loaded = false;
    this->gui.init();
    this->show_fps = true;
    this->fov = 30.0;
    this->hit_distance = 0.00005;
    this->max_ray_len = 200.0;
    printf(
            "<TAB>  Fullscreen\n"
            "<F>    Gui\n"
            "<R>    Reload shader\n"
            );
}

void RMSB::quit() {
    printf("%s\n", __func__);
 
    this->gui.quit();
    CloseWindow();

}

void RMSB::toggle_fullscreen() {
    ToggleBorderlessWindowed();
    
    int win_width = 0;
    int win_height = 0;

    if(!m_fullscreen) {
        m_fullscreen = true;
        int current_mon = GetCurrentMonitor();
        win_width = GetMonitorWidth(current_mon);
        win_height = GetMonitorHeight(current_mon);
    }
    else {
        m_fullscreen = false;
        win_width = m_winsize_nf.x;
        win_height = m_winsize_nf.y;
    }

    m_winsize_nf = (Vector2){ (float)GetScreenWidth(), (float)GetScreenHeight() };
    SetWindowSize(win_width, win_height);
}

void RMSB::update() {
    if(!this->time_paused) {
        this->time += GetFrameTime() * this->time_mult;
    }
}


void RMSB::render_shader() {
    if(!this->shader_loaded) {
        return;
    }

    const float ftime = (float)this->time;
    Vector2 screen_size = (Vector2) {
        GetScreenWidth(), GetScreenHeight()
    };

    BeginShaderMode(this->shader);
    SetShaderValue(this->shader, GetShaderLocation(this->shader, "time"), &ftime, SHADER_UNIFORM_FLOAT);
    SetShaderValue(this->shader, GetShaderLocation(this->shader, "FOV"), &this->fov, SHADER_UNIFORM_FLOAT);
    SetShaderValue(this->shader, GetShaderLocation(this->shader, "HIT_DISTANCE"), &this->hit_distance, SHADER_UNIFORM_FLOAT);
    SetShaderValue(this->shader, GetShaderLocation(this->shader, "MAX_RAY_LENGTH"), &this->max_ray_len, SHADER_UNIFORM_FLOAT);
    SetShaderValueV(this->shader, GetShaderLocation(this->shader, "screen_size"), &screen_size, SHADER_UNIFORM_VEC2, 1);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
    EndShaderMode();
}


void RMSB::reload_shader() {
    if(this->shader_loaded)  {
        UnloadShader(this->shader);
        this->shader_loaded = false;
        this->shader.id = 0;
    }

    const char* file = this->shader_filepath.c_str();
    if(!FileExists(file)) {
        fprintf(stderr, "'%s': \"%s\" does not exists\n",
                __func__, file);
        return;
    }

    char* data = LoadFileText(file);
    std::string code = InternalLib::get_instance().get_source() + data;
   
    this->shader = LoadShaderFromMemory(0, code.c_str() + '\0');
    printf("\033[90m%s\033[0m\n", code.c_str());
    printf("%i\n", this->shader.id);

    if(this->shader.id > 0) {
        this->shader_loaded = true;
    }

    UnloadFileText(data);


}
        

