#include "rmsb.hpp"


#include <stdio.h>



void key_inputs(RMSB* rmsb) {
    if(IsKeyPressed(KEY_TAB)) {
        rmsb->toggle_fullscreen();
    }

    if(IsKeyPressed(KEY_F)) {
        rmsb->gui.open = !rmsb->gui.open;
    }
   
    if(IsKeyPressed(KEY_R)) {
        rmsb->reload_shader();
    }


}


void loop(RMSB* rmsb) {

    while(!WindowShouldClose()) {
        key_inputs(rmsb);
        BeginDrawing();
        ClearBackground((Color){ 10, 10, 10, 255 });
       
        rmsb->render_shader();
        
        rmsb->update();
        rmsb->render_infolog();
        rmsb->gui.update();
        rmsb->gui.render(rmsb);

       
        if(rmsb->show_fps) {
            DrawFPS(10, GetScreenHeight()-20);
        }

        EndDrawing();
    }
}


int main(int argc, char** argv) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s example.glsl\n", argv[0]);
        return 1;
    }

    const char* shader_filepath = argv[1];
    

    if(!FileExists(shader_filepath)) {
        fprintf(stderr, "File \"%s\" does not exist!\n", shader_filepath);
        return 1;
    }
    
    int file_size = GetFileLength(shader_filepath);
    if(file_size <= 0) {
        fprintf(stderr, "File \"%s\" is empty.\n", shader_filepath);
        return 1;
    }

    RMSB rmsb;
    rmsb.shader_filepath += shader_filepath;
    rmsb.init();

    InternalLib& ilib = InternalLib::get_instance();
    ilib.create_source();
    rmsb.reload_shader();

    loop(&rmsb);
    rmsb.quit();

    return 0;
}


