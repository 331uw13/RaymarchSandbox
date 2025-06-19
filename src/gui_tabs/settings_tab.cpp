
#include "settings_tab.hpp"

#include "../util.hpp"
#include "../rmsb.hpp"
#include "../imgui.h"

   

void SettingsTab::render(RMSB* rmsb) {
    ImGui::Text("File:'%s'", rmsb->shader_filepath.c_str());
    if(ImGui::Button("Reload")) {
        rmsb->reload_shader();
    }

    ImGui::Checkbox("View functions", &rmsb->gui.view_functions);
    ImGui::Checkbox("Show FPS", &rmsb->show_fps);
    ImGui::Checkbox("Show Infolog", &rmsb->show_infolog);
    ImGui::Checkbox("Show Editor", &Editor::get_instance().open);
    
    //ImGui::SeparatorText("Render settings");

    if(ImGui::CollapsingHeader("Render Settings")) {

        ImGui::SliderFloat("##FIELD_OF_VIEW", 
                &rmsb->fov, 10.0, 120.0,
                "Field of view: %f");

        ImGui::SliderFloat("##HIT_DISTANCE",
                &rmsb->hit_distance, 0.000001, 0.001,
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

    if(ImGui::CollapsingHeader("Time Settings")) {
        
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

    if(ImGui::CollapsingHeader("Camera Settings")) {
        ImGui::SliderFloat("##CAMERA_SENSETIVITY",
                &rmsb->camera.sensetivity, 0.001, 1.0,
                "Sensetivity: %0.3f");
        
        if(ImGui::Button("Reset position")) {
            rmsb->camera.pos = (Vector3){ 0, 0, 0 };
        }

        ImGui::Separator();
    }

}


