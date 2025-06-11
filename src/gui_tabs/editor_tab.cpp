
#include "editor_tab.hpp"

#include "../util.hpp"
#include "../rmsb.hpp"
#include "../imgui.h"




void EditorSettingsTab::render(RMSB* rmsb) {

    Editor& editor = Editor::get_instance();


    ImGui::Text("For arrow keys, enter and backspace.");
    ImGui::SliderFloat("##EDITOR_KEY_REPEAT_SPEED", &editor.key_repeat_speed, 0.001, 0.5, "Key repeat speed: %0.4f");
    ImGui::SliderFloat("##EDITOR_KEY_REPEAT_DELAY", &editor.key_repeat_delay, 0.001, 0.5, "Key repeat delay: %0.4f");

}



