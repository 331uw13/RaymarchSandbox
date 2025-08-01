
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
#include "uniform_metadata.hpp"
#include "logfile.hpp"

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

static void raylib_message(int log_level, const char* text, va_list args) {

    #define BUF_SIZE 4096
    char buf[BUF_SIZE] = { 0 };
    vsnprintf(buf, BUF_SIZE, text, args);
    append_logfile(INFO, buf);
}

void GLAPIENTRY opengl_message(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity, 
        GLsizei length,
        const GLchar* message,
        const void* user_ptr
){
    if(type != GL_DEBUG_TYPE_ERROR) {
        return;
    }

    append_logfile(ERROR, "(OpenGL) ID=0%X, Severity=0x%X | %s", 
            id, severity, message);
}


void RMSB::load_resource_img(ImageIdx index, const char* path) {
    if(this->res.num_images >= RMSB_MAX_RESOURCE_IMAGES) {
        fprintf(stderr, "Cant fit more images to 'RMSB::res.images'... Resize the array!\n");
        return;
    }

    if(!FileExists(path)) {
        append_logfile(ERROR, "\"%s\": Does not exists.", path);
        return;
    }

    this->res.images[index].id = 0;
    this->res.images[index] = LoadTexture(path);
    if(this->res.images[index].id == 0) {
        append_logfile(ERROR, "\"%s\" Failed to load.", path);
    }
    this->res.num_images++;
}


void RMSB::load_resources() {
    
    load_resource_img(ImageIdx::EMPTY, "textures/empty.png");


}

void RMSB::init(const char* imgui_font_ttf, const char* editor_font_ttf) {
    this->running = true;

    SetTraceLogLevel(LOG_ALL);
    SetTraceLogCallback(raylib_message);
    
    InitWindow(
        DEFAULT_WIN_WIDTH,
        DEFAULT_WIN_HEIGHT,
        "Raymarch Sandbox"
    );

    SetWindowMinSize(GUI_WIDTH+FUNCTIONS_VIEW_WIDTH, 600);
    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
    SetExitKey(0);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(opengl_message, 0);

    this->load_resources();

    Editor& editor = Editor::get_instance();
    editor.init(editor_font_ttf);
    editor.load_file(this->shader_filepath);

    this->gui.init(imgui_font_ttf);

    m_infolog_size = 0;
    m_first_shader_load = true;
    m_pos_uniform_ptr = NULL;

    for(size_t i = 0; i < INFO_ARRAY_MAX_SIZE; i++) {
        m_infolog[i].enabled = 0;
    }

    this->translucent_step_size = 0.1;
    this->ao_step_size = 0.01;
    this->ao_num_samples = 32;
    this->ao_falloff = 3.0;
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
    this->ray_camera = (struct camera_t) {
        .pos = (Vector3){ 0, 0, 0 },
        .yaw = 0,
        .pitch = 0,
        .sensetivity = 0.065,
        .move_speed = 6
    };





    SetTargetFPS(this->fps_limit);
    
    ToggleBorderlessWindowed();

    int current_mon = GetCurrentMonitor();
    this->monitor_width = GetMonitorWidth(current_mon);
    this->monitor_height = GetMonitorHeight(current_mon);

    // Output shader to show the results.
    this->output_shader = load_shader_from_mem(OUT_VERTEX_SHADER_CODE, OUT_FRAGMENT_SHADER_CODE);
   
    /*
    // This is the texture everything is rendered on.
    this->render_texture = create_empty_texture(
            this->monitor_width,
            this->monitor_height,
            GL_RGBA16F);
    */
}

Texture RMSB::create_empty_texture(int width, int height, int format) {
    Texture tex;
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

        
void RMSB::delete_texture(Texture* tex) {
    if(tex->id > 0) {
        glDeleteTextures(1, &tex->id);
        tex->id = 0;
        printf("%s\n", __func__);
    }
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

    this->delete_texture(&this->render_texture);

    for(uint16_t i = 0; i < this->res.num_images; i++) {
        this->delete_texture(&this->res.images[i]);
    }

    this->gui.quit();
    CloseWindow();

}

void RMSB::update_camera() {
    const float dt = GetFrameTime();
    Vector2 md = GetMouseDelta();

    this->ray_camera.yaw -= (md.x * this->ray_camera.sensetivity) * (M_PI/180.0);
    this->ray_camera.pitch += (md.y * this->ray_camera.sensetivity) * (M_PI/180.0);

    Vector3 cam_dir = (Vector3) {
        (float)cos(this->ray_camera.yaw+(M_PI/2)) * cos(-this->ray_camera.pitch),
        sin(-this->ray_camera.pitch),
        (float)sin(this->ray_camera.yaw+(M_PI/2)) * cos(-this->ray_camera.pitch)
    };

    const float speed = dt * this->ray_camera.move_speed;

    if(IsKeyDown(KEY_W)) {
        this->ray_camera.pos.x += cam_dir.x * speed;
        this->ray_camera.pos.y += cam_dir.y * speed;
        this->ray_camera.pos.z += cam_dir.z * speed;
    }
    else
    if(IsKeyDown(KEY_S)) {
        this->ray_camera.pos.x -= cam_dir.x * speed;
        this->ray_camera.pos.y -= cam_dir.y * speed;
        this->ray_camera.pos.z -= cam_dir.z * speed;
    }


    Vector3 right = Vector3CrossProduct(cam_dir, (Vector3){ 0, 1, 0 });

    if(IsKeyDown(KEY_A)) {
        this->ray_camera.pos.x += right.x * speed;
        this->ray_camera.pos.z += right.z * speed;
    }
    else
    if(IsKeyDown(KEY_D)) {
        this->ray_camera.pos.x -= right.x * speed;
        this->ray_camera.pos.z -= right.z * speed;
    }

    if(IsKeyDown(KEY_SPACE)) {
        this->ray_camera.pos.y += speed;
    }
    if(IsKeyDown(KEY_LEFT_SHIFT)) {
        this->ray_camera.pos.y -= speed;
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


    int num_tex = 0;
    
    InternalLib& ilib = InternalLib::get_instance();
    glUseProgram(this->compute_shader);

    int texN[16] = { 0 };

    for(Uniform& u : ilib.uniforms) {

        switch(u.type) {
            case UniformDataType::RGBA:
                shader_uniform_vec4(compute_shader, u.name.c_str(),
                        (Vector4){ u.values[0], u.values[1], u.values[2], u.values[3] });
                break;

            case UniformDataType::XYZ:
                shader_uniform_vec3(compute_shader, u.name.c_str(),
                        (Vector3){ -u.values[0], u.values[1], u.values[2] });
                break;

            case UniformDataType::SINGLE:
                shader_uniform_float(compute_shader, u.name.c_str(), u.values[0]);
                break;

            case UniformDataType::TEXTURE:
                if(u.has_texture) {
                    glActiveTexture(GL_TEXTURE0+num_tex);
                    glBindTexture(GL_TEXTURE_2D, u.texture.id);
                    texN[num_tex] = num_tex;
                    num_tex++;
                    
                    u.texid_for_user = num_tex;
                }
                break;

            case UniformDataType::INVALID:break;
            case UniformDataType::NUM_TYPES:break;
            default:break;
        }
    }

    if(num_tex > 0) {
        glUniform1iv(
                glGetUniformLocation(compute_shader, "TEXTURES"),
                num_tex,
                texN
                );
    }

    shader_uniform_float(compute_shader, "time", ftime);
    shader_uniform_float(compute_shader, "FOV", this->fov);
    shader_uniform_float(compute_shader, "HIT_DISTANCE", this->hit_distance);
    shader_uniform_float(compute_shader, "MAX_RAY_LENGTH", this->max_ray_len);
    shader_uniform_vec2(compute_shader, "monitor_size", monitor_size);
    shader_uniform_vec3(compute_shader, "CameraInputPosition", this->ray_camera.pos);
    shader_uniform_float(compute_shader, "CAMERA_INPUT_YAW", this->ray_camera.yaw);
    shader_uniform_float(compute_shader, "CAMERA_INPUT_PITCH", this->ray_camera.pitch);
    shader_uniform_float(compute_shader, "TRANSLUCENT_STEP_SIZE", this->translucent_step_size);
    shader_uniform_float(compute_shader, "AO_STEP_SIZE", this->ao_step_size);
    shader_uniform_int(compute_shader, "AO_NUM_SAMPLES", this->ao_num_samples);
    shader_uniform_float(compute_shader, "AO_FALLOFF", this->ao_falloff);


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

void RMSB::reload_shader() {

    // Reset shader uniform locations.
    // New ones may be added or they maybe have changed.
    shader_util_reset_locations();
    
    ErrorLog& error_log = ErrorLog::get_instance();
    Editor& editor = Editor::get_instance();
    
    std::string shader_code = editor.get_content();
    error_log.clear();
   

    if(m_first_shader_load) {
        UniformMetadata::read(shader_code);
    }
   
    // TODO: Maybe should be removed once at startup so this dont need
    // to be called everytime the shader gets reloaded
    UniformMetadata::remove(&shader_code);


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
        /*
        fprintf(stderr,
                "-----------------------\n"
                "[ERROR]: \"%s\" Failed to compile...\n"
                "======================================================\n"
                "%s\n"
                "=================================================[End]\n"
                ,
                shader_filepath.c_str(),
                code.c_str()
                );*/

        error_log.get_error_position(&editor.error_row, &editor.error_column);
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

void RMSB::reload_state() {
    this->reload_lib();
    Editor::get_instance().clear_undo_stack();
    m_first_shader_load = true;

    this->ray_camera.pos = (Vector3){ 0, 0, 0 };
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
    int Y = 10;

    if(this->gui.open) {
        X += GUI_WIDTH;
    }
    if(FileBrowser::Instance().is_open()) {
        X += FILEBROWSER_WIDTH;
    }


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

void RMSB::render_3d() {

    this->raster_camera.position = this->ray_camera.pos;
    this->raster_camera.position.x = -this->raster_camera.position.x;
    this->raster_camera.up = (Vector3){ 0, 1, 0 };
    this->raster_camera.fovy = this->fov;
    this->raster_camera.projection = CAMERA_PERSPECTIVE;

    float y = this->ray_camera.yaw;
    float p = -this->ray_camera.pitch;

    Vector3 dir = (Vector3) {
        cos(p) * sin(y),
        sin(p),
        cos(p) * cos(y)
    };

    this->raster_camera.target = Vector3Add(this->raster_camera.position, dir);
   

    // For test render a sphere.
    BeginMode3D(this->raster_camera);

    if(m_pos_uniform_ptr) {
        edit_position_uniform();
    }

    EndMode3D();
}
        
void RMSB::edit_position_uniform() {
    if(!m_pos_uniform_ptr) { return; }

    constexpr float S = 1.0; // How far away the drag points are from origin.
    constexpr float   active_hit_radius = 1.0;
    float hit_radius = 0.3; // Default bounding sphere radius for ray.
    
    Vector3 origin = (Vector3) {
        m_pos_uniform_ptr->values[0], 
        m_pos_uniform_ptr->values[1], 
        m_pos_uniform_ptr->values[2]
    };

    constexpr uint32_t X_axis_i = 0;
    constexpr uint32_t Y_axis_i = 1;
    constexpr uint32_t Z_axis_i = 2;
    
    int32_t hit_axis_i = -1;
    
    const Vector3 axis_points[3] = {
        (Vector3) {  // X
            origin.x + S,
            origin.y,
            origin.z
        },
        (Vector3) {  // Y
            origin.x,
            origin.y + S,
            origin.z
        },
        (Vector3) {  // Z
            origin.x,
            origin.y,
            origin.z + S
        },
    };

    Vector3 hit_pos;


    if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        const Ray ray = GetScreenToWorldRay(GetMousePosition(), this->raster_camera);

        if(m_user_hold_uniform_pos) {
            hit_radius = active_hit_radius;
            RayCollision collision 
                = GetRayCollisionSphere(
                    ray,
                    axis_points[m_user_hold_axis_i],
                    hit_radius);

            if(collision.hit) {
                hit_pos = collision.point;
                hit_axis_i = m_user_hold_axis_i;
            }
        }
        else {
            for(uint32_t i = 0; i < 3; i++) {
                RayCollision collision = GetRayCollisionSphere(ray, axis_points[i], hit_radius);
                if(collision.hit) {
                    hit_axis_i = i;
                    hit_pos = collision.point;
                    m_user_hold_uniform_pos = true;
                    m_user_hold_axis_i = i;
                    break;
                }
            }
        }
    }
    else {
        m_user_hold_uniform_pos = false;
    }

    DrawSphere(origin, 0.05, WHITE);
    DrawSphere(axis_points[X_axis_i], 0.07, RED);
    DrawSphere(axis_points[Y_axis_i], 0.07, GREEN);
    DrawSphere(axis_points[Z_axis_i], 0.07, BLUE);
  
    constexpr float active_r = 0.2;
    constexpr float inactive_r = 0.125;

    DrawSphere(axis_points[X_axis_i],
            (hit_axis_i==X_axis_i) ? active_r : inactive_r, (Color){ 200, 30, 30, 100 });

    DrawSphere(axis_points[Y_axis_i],
            (hit_axis_i==Y_axis_i) ? active_r : inactive_r, (Color){ 30, 200, 30, 100 });
    
    DrawSphere(axis_points[Z_axis_i],
            (hit_axis_i==Z_axis_i) ? active_r : inactive_r, (Color){ 30, 30, 200, 100 });

    DrawCylinderEx(
            origin,
            axis_points[X_axis_i],
            0.05, 0.1, 4, (Color){ 200, 30, 30, 200 });

    DrawCylinderEx(
            origin,
            axis_points[Y_axis_i],
            0.05, 0.1, 4, (Color){ 30, 200, 30, 200 });
    
    DrawCylinderEx(
            origin,
            axis_points[Z_axis_i],
            0.05, 0.1, 4, (Color){ 30, 30, 200, 200 });


    if(hit_axis_i >= 0) {
        if(hit_axis_i == X_axis_i) {
            m_pos_uniform_ptr->values[0] = hit_pos.x - S/2;
        }
        else
        if(hit_axis_i == Y_axis_i) {
            m_pos_uniform_ptr->values[1] = hit_pos.y - S;
        }
        else
        if(hit_axis_i == Z_axis_i) {
            m_pos_uniform_ptr->values[2] = hit_pos.z - S/2;
        }
    }

}

void RMSB::set_position_uniform_ptr(Uniform* ptr) {
    m_pos_uniform_ptr = ptr;
}



