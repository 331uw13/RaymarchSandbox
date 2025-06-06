#include <raylib.h>
#include <stdio.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "rmsb.hpp"



static bool g_listen_shader_log = false;



static void tracelog_callback(int log_level, const char* text, va_list args) {
#define BUF_SIZE 4096
    char buf[BUF_SIZE] = { 0 };
    vsnprintf(buf, BUF_SIZE, text, args);
    puts(buf);

    if(g_listen_shader_log
    && (log_level == LOG_WARNING || log_level == LOG_ERROR)) {
        ErrorLog& log = ErrorLog::get_instance();
        log.add(buf);
    }

}

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
    SetExitKey(0);

    m_winsize_nf = (Vector2){ DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT };
    m_fullscreen = false;
    m_infolog_size = 0;
    m_first_shader_load = true;

    for(size_t i = 0; i < INFO_ARRAY_MAX_SIZE; i++) {
        m_infolog[i].enabled = 0;
    }

    this->show_infolog = true;
    this->reset_time_on_reload = true;
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

    SetTraceLogCallback(tracelog_callback);
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
        (float)GetScreenWidth(), (float)GetScreenHeight()
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
   
    ErrorLog::get_instance().clear();
    g_listen_shader_log = true;

    char* data = LoadFileText(file);
    std::string code = InternalLib::get_instance().get_source() + data;
    code += '\0';

    this->shader = LoadShaderFromMemory(0, code.c_str());
    //printf("%i\n", this->shader.id);
    printf("\033[90m%s\033[0m\n", code.c_str());

    if(this->shader.id > 0) {
        this->shader_loaded = true;
    }

    UnloadFileText(data);

    if(this->reset_time_on_reload) {
        this->time = 0;
    }
    g_listen_shader_log = false;

    if(IsShaderValid(this->shader)) {
        loginfo(GREEN, !m_first_shader_load ? "Shader Reloaded." : "Shader Loaded.");
        m_first_shader_load = false;
    }
}

void RMSB::loginfo(Color color, const char* text, ...) {
#define LOGINFO_BUF_SIZE 64
    va_list args;
    va_start(args, text);

    if(m_infolog_size+1 >= INFO_ARRAY_MAX_SIZE) {
        fprintf(stderr, "'%s': Info array is full\n",
                __func__);
        return;
    }

    
    char buf[LOGINFO_BUF_SIZE] = { 0 };
    vsnprintf(buf, LOGINFO_BUF_SIZE, text, args);

    va_end(args);


    struct infotext_t info = (struct infotext_t) {
        .data = buf,
        .color = color,
        .timer = 1.0,
        .enabled = 1
    };


    // Shift to right
    for(size_t i = INFO_ARRAY_MAX_SIZE-1; i > 0; i--) {
        m_infolog[i] = m_infolog[i-1];
    }

    m_infolog[0] = info;
}
        

void RMSB::render_infolog() {
    if(!this->show_infolog) { 
        return;
    }
    int X = 10;
    int Y = (this->gui.open ? GUI_HEIGHT : 0) + 10;


    struct infotext_t* info = NULL;
    for(size_t i = 0; i < INFO_ARRAY_MAX_SIZE; i++) {
        info = &m_infolog[i];
        if(!info->enabled) {
            continue;
        }

        info->color.a = info->timer * 255;
        DrawText(info->data.c_str(), X, Y, 20, info->color);

        Y += 25;

        info->timer -= GetFrameTime() * 0.5;
        if(info->timer < 0.0) {
            info->enabled = 0;
        }
    }
}


