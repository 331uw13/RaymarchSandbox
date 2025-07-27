
#include "settings_tab.hpp"

#include "../util.hpp"
#include "../rmsb.hpp"
#include "../imgui.h"


static constexpr ImVec4 RAY_SETTN_COLOR = ImVec4(1.0, 0.5, 0.7, 1.0);
static constexpr ImVec4 AO_SETTN_COLOR = ImVec4(0.5, 1.0, 0.5, 1.0);
static constexpr ImVec4 TR_SETTN_COLOR = ImVec4(0.5, 0.8, 1.0, 1.0);



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

    if(ImGui::Button("Reload Shader")) {
        rmsb->reload_shader();
    }

    if(ImGui::Button("Reload Lib")) {
        rmsb->reload_lib();
    }

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
                "%f");
        ImGui::SameLine();
        ImGui::TextColored(RAY_SETTN_COLOR, "- Field of view");


        ImGui::SliderFloat("##HIT_DISTANCE",
                &rmsb->hit_distance, 0.0001, 0.01,
                "%f");
        ImGui::SameLine();
        ImGui::TextColored(RAY_SETTN_COLOR, "- Hit distance");

        ImGui::SliderFloat("##RAY_LENGTH",
                &rmsb->max_ray_len, 10.0, 3000.0,
                "%0.2f");
        ImGui::SameLine();
        ImGui::TextColored(RAY_SETTN_COLOR, "- Ray length");



        ImGui::SliderFloat("##AO_STEP_SIZE",
                &rmsb->ao_step_size, 0.0008, 0.01,
                "%f");
        ImGui::SameLine();
        ImGui::TextColored(AO_SETTN_COLOR, "- AO step");

        ImGui::SliderInt("##AO_SAMPLES",
                &rmsb->ao_num_samples, 8, 128,
                "%i");
        ImGui::SameLine();
        ImGui::TextColored(AO_SETTN_COLOR, "- AO samples");

        ImGui::SliderFloat("##AO_FALLOFF",
                &rmsb->ao_falloff, 1.0, 5.0,
                "%f");
        ImGui::SameLine();
        ImGui::TextColored(AO_SETTN_COLOR, "- AO falloff");



        ImGui::SliderFloat("##TRANSLUCENT_STEP",
                &rmsb->translucent_step_size, 0.001, 0.2,
                "%f");
        ImGui::SameLine();
        ImGui::TextColored(TR_SETTN_COLOR, "- Translucent step");



        if(ImGui::SliderInt("##FPS_LIMIT",
                &rmsb->fps_limit, 30, 1000,
                "%i")) {
            SetTargetFPS(rmsb->fps_limit);
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.8, 0.8, 0.8, 1.0), "- FPS limit");



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
        
        ImGui::TextColored(ImVec4(1.0, 0.3, 0.3, 1.0), "X:%0.3f ", rmsb->camera.pos.x);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.3, 1.0, 0.3, 1.0), "Y:%0.3f ", rmsb->camera.pos.y);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.4, 0.4, 1.0, 1.0), "Z:%0.3f ", rmsb->camera.pos.z);

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


