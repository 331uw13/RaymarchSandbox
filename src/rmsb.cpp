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
    
    this->gui.init();

    Editor::get_instance().init();

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
    this->show_fps = true;
    this->fov = 60.0;
    this->hit_distance = 0.000015;
    this->max_ray_len = 500.0;
    printf(
            "Controls\n"
            "<CTRL + TAB>  Fullscreen\n"
            "<CTRL + F>    Gui\n"
            "<CTRL + R>    Reload shader\n"
            );


    char* data = LoadFileText(this->shader_filepath.c_str());
    const std::string shader_code = data;
    UnloadFileText(data);
    printf("%s\n", shader_code.c_str());
    Editor::get_instance().load_data(shader_code);

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

    InternalLib& ilib = InternalLib::get_instance();
    for(struct uniform_t u : ilib.uniforms) {

        switch(u.type) {
            case UNIFORM_TYPE_COLOR:
                SetShaderValue(this->shader, GetShaderLocation(this->shader, u.name.c_str()),
                        u.values, SHADER_UNIFORM_VEC4);
                break;
            case UNIFORM_TYPE_VALUE:
                SetShaderValue(this->shader, GetShaderLocation(this->shader, u.name.c_str()),
                        &u.values[0], SHADER_UNIFORM_FLOAT);
                break;
            case UNIFORM_TYPE_POSITION:
                break;
        }

    }

    SetShaderValue(this->shader, GetShaderLocation(this->shader, "time"), &ftime, SHADER_UNIFORM_FLOAT);
    SetShaderValue(this->shader, GetShaderLocation(this->shader, "FOV"), &this->fov, SHADER_UNIFORM_FLOAT);
    SetShaderValue(this->shader, GetShaderLocation(this->shader, "HIT_DISTANCE"), &this->hit_distance, SHADER_UNIFORM_FLOAT);
    SetShaderValue(this->shader, GetShaderLocation(this->shader, "MAX_RAY_LENGTH"), &this->max_ray_len, SHADER_UNIFORM_FLOAT);
    SetShaderValueV(this->shader, GetShaderLocation(this->shader, "screen_size"), &screen_size, SHADER_UNIFORM_VEC2, 1);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
    EndShaderMode();
}
        

void RMSB::proccess_shader_startup_cmd_line(const std::string* code_line) {

    if(code_line->size() < 4) { // Empty or just garbage.
        return;
    }

    if(!code_line->find("ADD:")) {
        fprintf(stderr, "'%s': \"%s\" not valid startup command.\n",
                __func__, code_line->c_str());
        return;
    }

    // TODO: Make this safer.
    //
#define COMMAND_ARGS_SIZE 16 
    std::string part = "";
    std::string args[COMMAND_ARGS_SIZE];
    size_t num_args = 0;

    // Read into 'parts'
    // and when <space char> is found. move it into args[num_args].
    for(size_t i = 0; i < code_line->size(); i++) {
        char c = (*code_line)[i];

        bool line_end = (c == ';');

        if((c == 0x20) || line_end) {
            args[num_args] = part;
            num_args++;
            part.clear();
            continue;
        }

        part += c;
    }


    // TODO: Use hash for string and switch statement if adding alot of types.


    std::string* type = &args[1];
    std::string* name = &args[2];

    printf("NAME:'%s'\n", name->c_str());

    struct uniform_t uniform = {
        .type = -1,
        .location = 0,
        .values = { 0.99, 0.56, 0.5, 1.0 },
        .name = *name
    };

    if(*type == "COLOR") {
        uniform.type = UNIFORM_TYPE_COLOR;

    }
    else
    if(*type == "VALUE") {
        uniform.type = UNIFORM_TYPE_VALUE;
    }
    else
    if(*type == "POSITION") {
        uniform.type = UNIFORM_TYPE_POSITION; 
    }


    InternalLib::get_instance().add_uniform(&uniform);

}

void RMSB::run_shader_startup_cmd(const std::string* shader_code) {
    size_t begin_index = shader_code->find(STARTUP_CMD_BEGIN_TAG);
    size_t end_index = shader_code->find(STARTUP_CMD_END_TAG);

    if((begin_index == std::string::npos) || (end_index == std::string::npos)) {
        printf("'%s': No valid startup command region found.\n", __func__);
        return;
    }

    begin_index += strlen(STARTUP_CMD_BEGIN_TAG)+1;

    printf("%s\n", __func__);

    std::string line = "";

    for(size_t i = begin_index; i < end_index; i++) {
        char c = (*shader_code)[i];
        if(c == '\n') {
            printf("\"%s\"\n", line.c_str());
            
            proccess_shader_startup_cmd_line(&line);

            line.clear();
            continue;
        }

        line += c;
    }


}

void RMSB::remove_startup_cmd_blocks(std::string* shader_code) {
    size_t begin_index = shader_code->find(STARTUP_CMD_BEGIN_TAG);
    size_t end_index = shader_code->find(STARTUP_CMD_END_TAG);

    if((begin_index == std::string::npos) || (end_index == std::string::npos)) {
        printf("'%s': No valid startup command region found.\n", __func__);
        return;
    }

    shader_code->erase(begin_index, end_index);
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
   
    g_listen_shader_log = true;

    // Load shader code from file.
    //char* data = LoadFileText(file);
    //std::string shader_code = data;
    //UnloadFileText(data);
    
    
    std::string shader_code = Editor::get_instance().get_data_str();


    if(m_first_shader_load) {
        run_shader_startup_cmd(&shader_code);
    }

    remove_startup_cmd_blocks(&shader_code);

    // Get internal library code.
    std::string code = InternalLib::get_instance().get_source() + shader_code;// + data;
    code += '\0';

    // Load shader.
    this->shader = LoadShaderFromMemory(0, code.c_str());
    printf("\033[90m%s\033[0m\n", code.c_str());

    if(this->shader.id > 0) {
        this->shader_loaded = true;
    }

    if(this->reset_time_on_reload) {
        this->time = 0;
    }
    g_listen_shader_log = false;

    // Message.
    if(IsShaderValid(this->shader)) {
        loginfo(GREEN, !m_first_shader_load ? "Shader Reloaded." : "Shader Loaded.");
        m_first_shader_load = false;
    }
    else {
        loginfo(RED, "Compiling or linking failed.");
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


