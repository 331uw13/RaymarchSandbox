#include <stdio.h>
#include <cmath>
#include "rmsb.hpp"
#include "input.hpp"
#include "logfile.hpp"

#include "config.hpp"
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


void init_all(RMSB* rmsb) {

    INIReader ini_reader(CONFIG_FILE);
    if(ini_reader.ParseError() < 0) {
        fprintf(stderr, "%s: Failed to load '%s'\n",
                __func__, CONFIG_FILE);
        exit(EXIT_FAILURE);
    }


    Editor& editor = Editor::get_instance();
    InternalLib& ilib = InternalLib::get_instance();
    
    Config::Settings settings;
    Config::read_values_before_init(ini_reader, &settings);

    printf("ImGui Font:  '%s'\n", settings.imgui_font.c_str());
    printf("Editor Font: '%s'\n", settings.editor_font.c_str());

    rmsb->init(
            settings.imgui_font.c_str(),
            settings.editor_font.c_str()
            );
    
    Config::read_values_after_init(ini_reader, rmsb);

    ilib.create_source();
    rmsb->reload_shader();
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
    RMSB rmsb;
    rmsb.shader_filepath = shader_filepath;
    init_all(&rmsb);
    editor.title = shader_filepath;
    loop(&rmsb);

    Editor::get_instance().quit();
    rmsb.quit();
    close_logfile();

    return 0;
}


