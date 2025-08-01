#include <stdio.h>
#include <cmath>
#include "rmsb.hpp"
#include "input.hpp"
#include "logfile.hpp"

#include "libs/INIReader.h"
#include "libs/glad.h"


static constexpr const char* 
    CONFIG_FILE = "rmsb.ini";


void key_inputs(RMSB* rmsb) {

    InputHandler::update_input_key(rmsb);
    InputHandler::handle_all_mode(rmsb);

    switch(rmsb->mode) {

        case EDIT_MODE:
            InputHandler::handle_edit_mode(rmsb);
            break;
        
        case VIEW_MODE:
            InputHandler::handle_view_mode(rmsb);
            break;

        // ... More can be added if needed :)
    }
}


void loop(RMSB* rmsb) {

    while(!WindowShouldClose() && rmsb->running) {
        key_inputs(rmsb);
        BeginDrawing();
        ClearBackground((Color){ 10, 10, 10, 255 });
       
        rmsb->render_shader();
        
        rmsb->update();
        rmsb->render_infolog();
        rmsb->gui.update();
        rmsb->gui.render(rmsb);

        Editor& editor = Editor::get_instance();
        if(!rmsb->allow_camera_input) {
            editor.update(rmsb);
        }
        editor.render(rmsb);
        rmsb->render_3d();
       
        if(rmsb->show_fps) {
            DrawFPS(10, GetScreenHeight()-20);
        }


        rmsb->input_key = 0;
        EndDrawing();
    }
}

void create_template_shader(const char* shader_filepath) {
    
    const char* shader_template = 
        "\n"
        "Material map(vec3 p) {\n"
        "    return EmptyMaterial();\n"
        "}\n"
        "\n"
        "\n"
        "vec3 raycolor() {\n"
        "    return Mdiffuse(Ray.mat);\n"
        "}\n"
        "\n"
        "vec3 raycolor_translucent() {\n"
        "    return Mdiffuse(Ray.mat);\n"
        "}\n"
        "\n"
        "\n"
        "void entry() {\n"
        "}\n"
        ;

    SaveFileText(shader_filepath, (char*)shader_template);
}


void read_config(RMSB* rmsb) {

    INIReader reader(CONFIG_FILE);

    if(reader.ParseError() < 0) {
        fprintf(stderr, "%s: Failed to load '%s'\n",
                __func__, CONFIG_FILE);
        return;
    }

    // Note:
    // The errors happening with config file reading
    // should be displayed to the user with rmsb->loginfo 
    // and written to logfile.

    rmsb->fps_limit = reader.GetInteger(
            "render_settings",
            "fps_limit", 125);

    rmsb->fov = reader.GetReal(
            "render_settings",
            "fov", 60.0);  

    rmsb->hit_distance = reader.GetReal(
            "render_settings",
            "hit_distance", 0.001);
    
    rmsb->max_ray_len = reader.GetReal(
            "render_settings",
            "max_ray_length", 500.0);
    
    rmsb->ao_step_size = reader.GetReal(
            "render_settings",
            "ao_step", 0.01);
    
    rmsb->ao_num_samples = reader.GetInteger(
            "render_settings",
            "ao_samples", 32);
    
    rmsb->ao_falloff = reader.GetReal(
            "render_settings",
            "ao_falloff", 3.0);
    
    rmsb->translucent_step_size = reader.GetReal(
            "render_settings",
            "translucent_step", 0.1);


    std::string res_str = reader.GetString(
            "render_settings",
            "render_resolution", "");
    if(res_str.empty()) {
        rmsb->loginfo(RED, "Could not find render resolution setting! Set to 'HALF'");
        append_logfile(ERROR, "Could not find render resolution setting.");
        res_str = "HALF";
    }

    int res_x = rmsb->monitor_width;
    int res_y = rmsb->monitor_height;

    if(res_str == "HALF") {
        res_x /= 2;
        res_y /= 2;
    }
    else
    if(res_str == "LOW") {
        res_x /= 3;
        res_y /= 3;
    }
    if(res_str == "CUSTOM") {
        res_x = reader.GetInteger(
                "render_settings",
                "custom_render_resolution_X", 0);
        
        res_y = reader.GetInteger(
                "render_settings",
                "custom_render_resolution_Y", 0);

        if(res_x <= 0) {
            rmsb->loginfo(RED, "Custom resolution X is invalid, set to half.");
            append_logfile(ERROR, "Custom resolution X is invalid. Too small.");
            res_x = rmsb->monitor_width/2;
        }
        if(res_y <= 0) {
            rmsb->loginfo(RED, "Custom resolution Y is invalid, set to half.");
            append_logfile(ERROR, "Custom resolution Y is invalid. Too small.");
            res_y = rmsb->monitor_height/2;
        }

        if(res_x > rmsb->monitor_width) {
            res_x = rmsb->monitor_width;
        }
        if(res_y > rmsb->monitor_height) {
            res_y = rmsb->monitor_height;
        }
    }

    rmsb->render_texture = rmsb->create_empty_texture(
            res_x, res_y, GL_RGBA16F);
    
    SetTargetFPS(rmsb->fps_limit);
}

int main(int argc, char** argv) {

    if(argc != 2) {
        fprintf(stderr, 
                "\033[36m[RaymarchSandbox]\033[0m\n"
                "Usage: %s <shader.glsl>\n"
                "\033[90m> Already existing file is read, otherwise empty template is created\n"
                "\033[90m> To get started, reading examples/intro.glsl and other examples is recommended.\033[0m\n"
                , argv[0]);
        return 1;
    }

    assign_logfile("rmsb.log");
    const char* shader_filepath = argv[1];
    

    if(!FileExists(shader_filepath)) {
        create_template_shader(shader_filepath);
        append_logfile(INFO, "Shader \"%s\" did not exist. Created new template.",
                shader_filepath);
    }
    
    int file_size = GetFileLength(shader_filepath);
    if(file_size <= 0) {
        // This should not happen but just in case inform the user.
        append_logfile(ERROR, "Shader \"%s\" is empty.. Failed to create template?");
        return 1;
    }

    Editor& editor = Editor::get_instance();
    InternalLib& ilib = InternalLib::get_instance();

    RMSB rmsb;
    rmsb.shader_filepath = shader_filepath;
    rmsb.init();
    read_config(&rmsb);

    ilib.create_source();
    rmsb.reload_shader();

    editor.title = shader_filepath;

    loop(&rmsb);
    rmsb.quit();
    editor.quit();
    close_logfile();

    return 0;
}


