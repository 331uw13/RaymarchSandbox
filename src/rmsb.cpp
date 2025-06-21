#include <raylib.h>
#include <raymath.h>
#include <rcamera.h>

#include <stdio.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "rmsb.hpp"
#include "shader_util.hpp"



static const char* const VERTEX_SHADER_CODE =
"#version 430\n"
"in vec3 vertexPosition;\n"
"in vec2 vertexTexCoord;\n"
"in vec4 vertexColor;\n"
"uniform mat4 mvp;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
"}\n"
;


/*
static void tracelog_callback(int log_level, const char* text, va_list args) {
#define BUF_SIZE 4096
    char buf[BUF_SIZE] = { 0 };
    vsnprintf(buf, BUF_SIZE, text, args);
    puts(buf);
}
*/

void RMSB::init() {
    this->running = true;

    SetTraceLogLevel(LOG_NONE);
    
    InitWindow(
        DEFAULT_WIN_WIDTH,
        DEFAULT_WIN_HEIGHT,
        "Raymarch Sandbox"
    );
    SetWindowMinSize(200, 200);
    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
    SetExitKey(0);
   
    this->gui.init();

    Editor::get_instance().init();

    m_infolog_size = 0;
    m_first_shader_load = true;

    for(size_t i = 0; i < INFO_ARRAY_MAX_SIZE; i++) {
        m_infolog[i].enabled = 0;
    }

    this->fallback_user_shader = true;
    this->auto_reload = false;
    this->auto_reload_delay = 3.0;
    this->input_key = 0;
    this->mode = EDIT_MODE;
    this->fps_limit = 300;
    this->show_infolog = true;
    this->reset_time_on_reload = true;
    this->time_paused = false;
    this->time_mult = 1.0f;
    this->time = 0.0;
    this->file_read_timer = 0.0f;
    this->shader_loaded = false;
    this->show_fps = true;
    this->fov = 60.0;
    this->hit_distance = 0.000150;
    this->max_ray_len = 500.0;
    this->allow_camera_input = false; 
    this->camera = (struct camera_t) {
        .pos = (Vector3){ 0, 0, 0 },
        .dir = (Vector3){ 0, 0, 0 },
        .yaw = 0,
        .pitch = 0,
        .sensetivity = 0.1,
        .move_speed = 10
    };

    char* data = LoadFileText(this->shader_filepath.c_str());
    const std::string shader_code = data;
    UnloadFileText(data);
    Editor::get_instance().load_data(shader_code);

    SetTargetFPS(this->fps_limit);
    //SetTraceLogCallback(tracelog_callback);

    
    ToggleBorderlessWindowed();
}
        
void RMSB::quit() {
    printf("%s\n", __func__);

    if(this->shader.id > 0) {
        unload_shader(&this->shader);
    }

    this->gui.quit();
    CloseWindow();

}

void RMSB::update_camera() {
    const float dt = GetFrameTime();
    Vector2 md = GetMouseDelta();

    this->camera.yaw -= (md.x * this->camera.sensetivity) * (M_PI/180.0);
    this->camera.pitch += (md.y * this->camera.sensetivity) * (M_PI/180.0);

    Vector3 cam_dir = (Vector3) {
        (float)cos(this->camera.yaw+(M_PI/2)) * cos(-this->camera.pitch),
        sin(-this->camera.pitch),
        (float)sin(this->camera.yaw+(M_PI/2)) * cos(-this->camera.pitch)
    };

    const float speed = dt * this->camera.move_speed;

    if(IsKeyDown(KEY_W)) {
        this->camera.pos.x += cam_dir.x * speed;
        this->camera.pos.y += cam_dir.y * speed;
        this->camera.pos.z += cam_dir.z * speed;
    }
    else
    if(IsKeyDown(KEY_S)) {
        this->camera.pos.x -= cam_dir.x * speed;
        this->camera.pos.y -= cam_dir.y * speed;
        this->camera.pos.z -= cam_dir.z * speed;
    }


    Vector3 right = Vector3CrossProduct(cam_dir, (Vector3){ 0, 1, 0 });

    if(IsKeyDown(KEY_A)) {
        this->camera.pos.x += right.x * speed;
        this->camera.pos.z += right.z * speed;
    }
    else
    if(IsKeyDown(KEY_D)) {
        this->camera.pos.x -= right.x * speed;
        this->camera.pos.z -= right.z * speed;
    }

    if(IsKeyDown(KEY_SPACE)) {
        this->camera.pos.y += speed;
    }
    if(IsKeyDown(KEY_LEFT_SHIFT)) {
        this->camera.pos.y -= speed;
    }
}

void RMSB::update() {
    if(!this->time_paused) {
        this->time += GetFrameTime() * this->time_mult;
    }

    if(this->allow_camera_input) {
        update_camera();
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
                // ... TODO
                break;
        }
    }

    shader_uniform_float(&this->shader, "time", &ftime);
    shader_uniform_float(&this->shader, "FOV", &this->fov);
    shader_uniform_float(&this->shader, "HIT_DISTANCE", &this->hit_distance);
    shader_uniform_float(&this->shader, "MAX_RAY_LENGTH", &this->max_ray_len);
    shader_uniform_vec2(&this->shader, "screen_size", &screen_size);
    shader_uniform_vec3(&this->shader, "CAMERA_INPUT_POS", &this->camera.pos);
    shader_uniform_float(&this->shader, "CAMERA_INPUT_YAW", &this->camera.yaw);
    shader_uniform_float(&this->shader, "CAMERA_INPUT_PITCH", &this->camera.pitch);

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), WHITE);
    EndShaderMode();
}
        

void RMSB::proccess_shader_startup_cmd_line(const std::string* code_line) {

    if(code_line->empty()) {
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

    //printf("NAME:'%s'\n", name->c_str());

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
    std::string line = "";

    for(size_t i = begin_index; i < end_index; i++) {
        char c = (*shader_code)[i];
        if(c == '\n') {
            
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



void RMSB::reload_shader(ReloadOption option) {
 
    if(option == USER_FALLBACK_OPTION) {
        if(this->fallback_user_shader) {
            option = FALLBACK_TO_CURRENT;
        }
        else {
            option = NO_FALLBACK;
        }
    }

    // Reset shader uniform locations.
    // New ones may be added or they maybe have changed.
    shader_util_reset_locations();
    
    ErrorLog::get_instance().clear();
    
   
    std::string shader_code = Editor::get_instance().get_content();
    
    // Add user specified uniforms from the file
    if(m_first_shader_load) {
        run_shader_startup_cmd(&shader_code);
    }

    remove_startup_cmd_blocks(&shader_code);


    // Merge user shader code and internal lib.
    std::string code = InternalLib::get_instance().get_source();
    code += shader_code;
    code += '\0';



    if(option == NO_FALLBACK) {
        if(this->shader.id > 0) {
            unload_shader(&this->shader);
        }
        this->shader = load_shader_from_mem(VERTEX_SHADER_CODE, code.c_str());
        this->shader_loaded = (this->shader.id > 0);
    }
    else
    if(option == FALLBACK_TO_CURRENT) {
        Shader tmp_shader = load_shader_from_mem(VERTEX_SHADER_CODE, code.c_str());
        if(tmp_shader.id > 0) {
            unload_shader(&this->shader);
            this->shader = tmp_shader;
            this->shader_loaded = true;
        }
        else {
            unload_shader(&tmp_shader);
        }
    }


    //this->shader = LoadShaderFromMemory(0, code.c_str());

    // 0 value for vertex shader will use raylib's default vertex shader.
    //Shader tmp_shader = LoadShaderFromMemory(0, code.c_str());
    /*
    if(IsShaderValid(this->shader)) {
        UnloadShader(this->shader);
    }
    */

    /*
    if(this->shader.id > 0) {
        unload_shader(&this->shader);
    }

    // Compile and link shader.
    this->shader = load_shader_from_mem(VERTEX_SHADER_CODE, code.c_str());
    this->shader_loaded = (this->shader.id > 0);
    */


    /*
    if(option == NO_FALLBACK) {
        if(IsShaderValid(this->shader)) {
            UnloadShader(this->shader);
        }
        this->shader = tmp_shader;
        this->shader_loaded = (this->shader.id > 0);
    }
    else
    if(option == FALLBACK_TO_CURRENT) {
        bool tmp_shader_valid = IsShaderValid(tmp_shader);
        if(tmp_shader_valid) {
            UnloadShader(this->shader);
            this->shader = tmp_shader;
        }
        else {
            loginfo(PURPLE, "Unloaded temporary shader");
            UnloadShader(tmp_shader);
        }
    }
    */

    // Tell user what happened.
    if(option == NO_FALLBACK) {
        if(IsShaderValid(this->shader)) {
            loginfo(GREEN, !m_first_shader_load ? "Shader Reloaded." : "Shader Loaded.");
        }
        else {
            loginfo(RED, "Shader failed to compile.");
        }
    }
    
    if(this->reset_time_on_reload) {
        this->time = 0;
    }
    
    m_first_shader_load = false;
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
    int X = (this->gui.open ? GUI_WIDTH : 0) + 10;
    int Y = 10;


    struct infotext_t* info = NULL;
    for(size_t i = 0; i < INFO_ARRAY_MAX_SIZE; i++) {
        info = &m_infolog[i];
        if(!info->enabled) {
            continue;
        }

        info->color.a = info->timer * 255;
        float box_width = MeasureText(info->data.c_str(), 20) + 10;
        DrawRectangle(X-5, Y-2, box_width, 22, (Color){ 0, 0, 0, (unsigned char)(info->color.a*0.5) });
        DrawText(info->data.c_str(), X+2, Y+2, 20, (Color){ 0, 0, 0, info->color.a });
        DrawText(info->data.c_str(), X, Y, 20, info->color);

        Y += 25;

        info->timer -= GetFrameTime() * 0.275;
        if(info->timer < 0.0) {
            info->enabled = 0;
        }
    }
}


