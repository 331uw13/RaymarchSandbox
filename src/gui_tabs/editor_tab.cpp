
#include "editor_tab.hpp"

#include "../util.hpp"
#include "../rmsb.hpp"
#include "../imgui.h"




void EditorSettingsTab::render(RMSB* rmsb) {

    Editor& editor = Editor::get_instance();


    ImGui::Text("For arrow keys, enter and backspace.");
    ImGui::SliderFloat("##EDITOR_KEY_REPEAT_SPEED", &editor.key_repeat_speed, 0.001, 0.5, "Key repeat speed: %0.4f");
    ImGui::SliderFloat("##EDITOR_KEY_REPEAT_DELAY", &editor.key_repeat_delay, 0.001, 0.5, "Key repeat delay: %0.4f");

    ImGui::Separator();

    ImGui::SliderInt("##EDITOR_OPACITY", &editor.opacity, 0, 255, "Opacity: %i");
    ImGui::SliderFloat("##EDITOR_DIFF_CHECK_DELAY",
            &editor.diff_check_delay, 1.0, 10.0, "Difference Check Delay: %0.1f");
}



