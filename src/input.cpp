#include "input.hpp"
#include "rmsb.hpp"
#include "editor.hpp"



void InputHandler::update_input_key(RMSB* rmsb) {
    for(int key = KEY_A; key < KEY_Z; key++) {
        if(IsKeyPressed(key)) {
            rmsb->input_key = key;
            break;
        }
    }
}


void InputHandler::handle_all_mode(RMSB* rmsb) {
    
    if(!IsKeyDown(KEY_LEFT_CONTROL)) {
        return;
    }
   
    Editor& editor = Editor::get_instance();

    switch(rmsb->input_key) {
    
        case KEY_X:
            if(rmsb->mode == VIEW_MODE) {
                rmsb->mode = EDIT_MODE;
                rmsb->loginfo((Color){ 0x39, 0xA8, 0x7E, 0xFF }, "-> Edit_Mode");
                rmsb->allow_camera_input = false;
                EnableCursor();
            }
            else
            if(rmsb->mode == EDIT_MODE) {
                rmsb->mode = VIEW_MODE;
                rmsb->loginfo((Color){ 0x39, 0x99, 0xA8, 0xFF }, "-> View_Mode");
            }
            break;

        case KEY_E:
            editor.open = !editor.open;
            break;

        case KEY_F:
            rmsb->gui.open = !rmsb->gui.open;
            break;
        
        case KEY_A:
            rmsb->gui.open = !rmsb->gui.open;
            editor.open = rmsb->gui.open;
            break;

        case KEY_R:
            rmsb->reload_shader(USER_FALLBACK_OPTION);
            break;


        default:break;
    }
}

void InputHandler::handle_view_mode(RMSB* rmsb) {
    
    if(!IsKeyDown(KEY_LEFT_CONTROL)) {
        return;
    }

    if(IsKeyPressed(KEY_C)) {
        rmsb->allow_camera_input = !rmsb->allow_camera_input;
        if(rmsb->allow_camera_input) {
            rmsb->loginfo((Color){ 0x26, 0xD7, 0xE0, 0xFF }, "Camera Input: Enabled");
            DisableCursor();
        }
        else {
            rmsb->loginfo((Color){ 0xCC, 0x60, 0xC7, 0xFF }, "Camera Input: Disabled");
            EnableCursor();
        }
    }
    
}


void InputHandler::handle_edit_mode(RMSB* rmsb) {
    int key = GetKeyPressed();
    if(key == 0) {
        return;
    }

    if(!IsKeyDown(KEY_LEFT_CONTROL)) {
        return;
    }

    if(IsKeyPressed(KEY_S)) {
        Editor::get_instance().save(rmsb->shader_filepath);
        rmsb->loginfo(GREEN, TextFormat("Shader Saved (%s)", rmsb->shader_filepath.c_str()));
    }
}


