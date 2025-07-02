
#include "settings_tab.hpp"

#include "../util.hpp"
#include "../rmsb.hpp"
#include "../imgui.h"


void SettingsTab::render(RMSB* rmsb) {

    if(ImGui::SmallButton(" Quit ")) {
       
        Editor& editor = Editor::get_instance();
        if(editor.content_changed) {
            int answer = rmsb->gui.ask_question(
                    TextFormat("Warning: shader \"%s\" is not saved.", rmsb->shader_filepath.c_str()),
                    { "Save and quit.", "Quit anyway!" });

            if(answer == 0) {
                editor.save(rmsb->shader_filepath.c_str());
            }
        }
        
        rmsb->running = false;
    }
    ImGui::SameLine();
    ImGui::Text("File: %s", rmsb->shader_filepath.c_str());

    ImGui::Separator();

    if(ImGui::Button("Reload")) {
        rmsb->reload_shader(USER_FALLBACK_OPTION);
    }

    ImGui::SameLine();
    ImGui::Checkbox("Fallback to working shader", &rmsb->fallback_user_shader);

    ImGui::Checkbox("Auto Reload", &rmsb->auto_reload);
    if(rmsb->auto_reload) {
        ImGui::SliderFloat("##AUTO_RELOAD_TIME",
                &rmsb->auto_reload_delay, 0.2, 10.0,
                "Auto Reload Time: %0.1f");
    }

    ImGui::Checkbox("View Functions", &rmsb->gui.view_functions);
    ImGui::Checkbox("Show FPS", &rmsb->show_fps);
    ImGui::Checkbox("Show Infolog", &rmsb->show_infolog);
    ImGui::Checkbox("Show Editor", &Editor::get_instance().open);


    //ImGui::SeparatorText("Render settings");

    if(ImGui::CollapsingHeader("Render Settings", ImGuiTreeNodeFlags_DefaultOpen)) {

        ImGui::SliderFloat("##FIELD_OF_VIEW", 
                &rmsb->fov, 10.0, 120.0,
                "Field of view: %f");

        ImGui::SliderFloat("##HIT_DISTANCE",
                &rmsb->hit_distance, 0.0001, 0.01,
                "Hit distance: %f");

        ImGui::SliderFloat("##MAX_RAY_LENGTH",
                &rmsb->max_ray_len, 10.0, 3000.0,
                "Max ray length: %0.2f");

        if(ImGui::SliderInt("##FPS_LIMIT",
                &rmsb->fps_limit, 10, 1000,
                "FPS Limit: %i")) {
            SetTargetFPS(rmsb->fps_limit);
        }

        ImGui::Separator();
    }

    if(ImGui::CollapsingHeader("Time Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        
        if(ImGui::Button("Reset Time")) {
            rmsb->time = 0.0;
        }
        ImGui::SameLine();
        ImGui::Checkbox("Pause", &rmsb->time_paused);
        ImGui::SameLine();
        ImGui::Text("| %0.2f", rmsb->time);
        
        if(ImGui::Button("Reset")) {
            rmsb->time_mult = 1.0f;
        }
        ImGui::SameLine();
        ImGui::SliderFloat("##GLOBAL_TIME_SCALE",
                &rmsb->time_mult, -10.0, 10.0,
                "Global time scale: %0.3f");

        ImGui::Checkbox("Reset time on reload", &rmsb->reset_time_on_reload);
        ImGui::Separator();
    }

    if(ImGui::CollapsingHeader("Camera Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("##CAMERA_SENSETIVITY",
                &rmsb->camera.sensetivity, 0.001, 1.0,
                "Sensetivity: %0.3f");
       
        ImGui::SliderFloat("##CAMERA_MOVEMENT_SPEED",
                &rmsb->camera.move_speed, 1.0, 60.0,
                "Movement Speed: %0.01f");

        if(ImGui::Button("Reset position")) {
            rmsb->camera.pos = (Vector3){ 0, 0, 0 };
        }
        
        if(ImGui::Button("Reset direction")) {
            rmsb->camera.yaw = 0;
            rmsb->camera.pitch = 0;
        }

        ImGui::Separator();
    }

}


