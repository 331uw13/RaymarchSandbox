
#include "libs/glad.h"
#include <raylib.h>

#include <raymath.h>
#include <rcamera.h>

#include <stdio.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "rmsb.hpp"
#include "shader_util.hpp"
#include "preproc.hpp"

#include <rlgl.h>


static const char* const OUT_VERTEX_SHADER_CODE =
GLSL_VERSION
"in vec3 vertexPosition;\n"
"in vec2 vertexTexCoord;\n"
"in vec4 vertexColor;\n"
"out vec2 texcoords;\n"
"uniform mat4 mvp;\n"
"\n"
"void main()\n"
"{\n"
"    texcoords = vertexTexCoord;\n"
"    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
"}\n"
;

static const char* const OUT_FRAGMENT_SHADER_CODE =
GLSL_VERSION

"in vec2 texcoords;\n"
"out vec4 out_color;"
"uniform sampler2D fuckshit;\n"
"uniform vec2 ures;\n"
"\n"
"void main()\n"
"{\n"
"    vec2 uv = (gl_FragCoord.xy) / ures;"
"    out_color = texture(fuckshit, uv);\n"
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
    SetWindowMinSize(GUI_WIDTH+FUNCTIONS_VIEW_WIDTH, 600);
    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
    SetExitKey(0);
   
    this->gui.init();

    Editor::get_instance().init();

    m_infolog_size = 0;
    m_first_shader_load = true;

    for(size_t i = 0; i < INFO_ARRAY_MAX_SIZE; i++) {
        m_infolog[i].enabled = 0;
    }

    this->translucent_step_size = 0.1;
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
    this->show_fps = true;
    this->fov = 60.0;
    this->hit_distance = 0.001000;
    this->max_ray_len = 1000.0;
    this->allow_camera_input = false; 
    this->camera = (struct camera_t) {
        .pos = (Vector3){ 0, 0, 0 },
        .dir = (Vector3){ 0, 0, 0 },
        .yaw = 0,
        .pitch = 0,
        .sensetivity = 0.065,
        .move_speed = 6
    };

    char* data = LoadFileText(this->shader_filepath.c_str());
    
    const std::string shader_code = data;
    UnloadFileText(data);
    
    Editor::get_instance().load_data(shader_code);

    SetTargetFPS(this->fps_limit);
    //SetTraceLogCallback(tracelog_callback);
    
    //ToggleBorderlessWindowed();

    int current_mon = GetCurrentMonitor();
    this->monitor_width = GetMonitorWidth(current_mon);
    this->monitor_height = GetMonitorHeight(current_mon);

    // Output shader to show the results.
    this->output_shader = load_shader_from_mem(OUT_VERTEX_SHADER_CODE, OUT_FRAGMENT_SHADER_CODE);
    
    // This is the texture everything is rendered on.
    this->render_texture = create_empty_texture(
            this->monitor_width,
            this->monitor_height,
            GL_RGBA16F);

}

struct texture_t RMSB::create_empty_texture(int width, int height, int format) {
    struct texture_t tex;
    tex.width = width;
    tex.height = height;
    tex.format = format;
    tex.id = 0;

	glGenTextures(1, &tex.id);
	glBindTexture(GL_TEXTURE_2D, tex.id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, format, tex.width, tex.height, 0, GL_RGBA, GL_FLOAT, NULL);

    printf("%s: %ix%i\n", __func__, width, height);

    return tex;
}

uint32_t RMSB::create_ssbo(int binding_point, size_t size) {
    uint32_t ssbo = 0;

    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    return ssbo;
}

void RMSB::quit() {
    printf("%s: %s\n", __FILE__, __func__);

    if(this->output_shader.id > 0) {
        unload_shader(&this->output_shader);
    }

    if(this->compute_shader > 0) {
        glDeleteProgram(this->compute_shader);
    }

    if(this->render_texture.id > 0) {
        glDeleteTextures(1, &this->render_texture.id);
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

    const float ftime = (float)this->time;
    Vector2 monitor_size = (Vector2) {
        (float)this->monitor_width, (float)this->monitor_height
    };

    
    InternalLib& ilib = InternalLib::get_instance();
    for(struct uniform_t u : ilib.uniforms) {

        switch(u.type) {
            case UNIFORM_TYPE_COLOR:
                shader_uniform_vec4(compute_shader, u.name.c_str(),
                        (Vector4){ u.values[0], u.values[1], u.values[2], u.values[3] });
                break;
            case UNIFORM_TYPE_VALUE:
                shader_uniform_float(compute_shader, u.name.c_str(), u.values[0]);
                //SetShaderValue(this->shader, GetShaderLocation(this->shader, u.name.c_str()),
                //        &u.values[0], SHADER_UNIFORM_FLOAT);
                break;
            case UNIFORM_TYPE_POSITION:
                // ... TODO
                break;
        }
    }
    shader_uniform_float(compute_shader, "time", ftime);
    shader_uniform_float(compute_shader, "FOV", this->fov);
    shader_uniform_float(compute_shader, "HIT_DISTANCE", this->hit_distance);
    shader_uniform_float(compute_shader, "MAX_RAY_LENGTH", this->max_ray_len);
    shader_uniform_vec2(compute_shader, "monitor_size", monitor_size);
    shader_uniform_vec3(compute_shader, "CameraInputPosition", this->camera.pos);
    shader_uniform_float(compute_shader, "CAMERA_INPUT_YAW", this->camera.yaw);
    shader_uniform_float(compute_shader, "CAMERA_INPUT_PITCH", this->camera.pitch);
    shader_uniform_float(compute_shader, "TRANSLUCENT_STEP_SIZE", this->translucent_step_size);
    
    glUseProgram(this->compute_shader);
	glBindImageTexture(
            8, // Binding point.
            this->render_texture.id,
            0,
            GL_FALSE,
            0,
            GL_WRITE_ONLY,
            this->render_texture.format
            );

    glDispatchCompute(this->render_texture.width / 8, this->render_texture.height / 8, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
   

    // Draw the results from the compute shader.

    BeginShaderMode(this->output_shader);
    shader_uniform_vec2(this->output_shader.id, "ures", monitor_size);
    rlEnableShader(this->output_shader.id);
    rlSetUniformSampler(GetShaderLocation(this->output_shader, "fuckshit"), this->render_texture.id);
   
    DrawRectangle(0, 0, this->monitor_width, this->monitor_height, RED);
    EndShaderMode();
}

void RMSB::get_cmdline_value(const std::string& code_line, float values[4]) {
    
    size_t array_start = code_line.find("(");
    size_t array_end = code_line.find(")");

    if((array_start == std::string::npos)
    || (array_end == std::string::npos)) {
        return;
    }


    char buf[64] = { 0 };
    size_t buf_i = 0;
    size_t values_i = 0;
    //printf("\n");

    for(size_t i = array_start+1; i < array_end+1; i++) {
        char c = code_line[i];
      
        if((c == ',') || (c == ')')) {
            // The value is written to buf,
            // convert to float and move it to values array.
            values[values_i] = atof(buf);
            memset(buf, 0, buf_i);
            buf_i = 0;
            values_i++;

            continue;
        } 
        
        if((c >= '0' && c <= '9') || (c == '.')) {
            buf[buf_i] = c;
            buf_i++;
            if(buf_i >= 64) {
                fprintf(stderr, "'%s': Invalid array for '%s'\n",
                        __func__, code_line.c_str());
                return;
            }
        }
       
    }
}

void RMSB::process_shader_startup_cmd_line(const std::string& code_line) {

    if(code_line.empty()) {
        return;
    }

    if(!code_line.find("ADD:")) {
        fprintf(stderr, "'%s': \"%s\" not valid startup command.\n",
                __func__, code_line.c_str());
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
    for(size_t i = 0; i < code_line.size(); i++) {
        char c = code_line[i];

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
        .values = { 0.0, 0.0, 0.0, 0.0 },
        .name = *name
    };

    get_cmdline_value(code_line, uniform.values);

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
            
            process_shader_startup_cmd_line(line);

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

    if(begin_index > 0) {
        begin_index--;
    }

    shader_code->erase(begin_index, end_index);
}



void RMSB::reload_shader() {

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
    std::string code = "";
    code += GLSL_VERSION;
    Preproc::process_glsl(&shader_code, &code);

    code += InternalLib::get_instance().get_source();
    code += shader_code;
    code.push_back('\0');


    if(this->compute_shader > 0) {
        glDeleteProgram(this->compute_shader);
    }
    this->compute_shader = load_compute_shader(code.c_str());

    // Tell user what happened.
    if(this->compute_shader > 0) {
        loginfo(GREEN, !m_first_shader_load ? "Shader Reloaded." : "Shader Loaded.");
    }
    else {
        loginfo(RED, "Shader failed to compile.");
    }
    
    if(this->reset_time_on_reload) {
        this->time = 0;
    }
    
    m_first_shader_load = false;
}
        

void RMSB::reload_lib() {
    InternalLib& ilib = InternalLib::get_instance();
    ilib.clear();
    ilib.create_source();
    m_first_shader_load = true;
    reload_shader();
    loginfo(PURPLE, "Internal library reloaded");
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


