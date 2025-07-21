
#include "keybinds_tab.hpp"

#include "../util.hpp"
#include "../rmsb.hpp"
#include "../imgui.h"


// TODO: Allow user to edit keybinds and save them.


static const char* const g_keybinds[][32] = {
    { "Switch between View and Edit mode", "CTRL + X", "(All)" },
    { "GLSL Editor", "CTRL + E", "(All)" },
    { "Toggle Editor And Gui", "CTRL + A", "(All)" },
    { "Reload shader", "CTRL + R", "(All)" },
    { "Reload internal.glsl", "CTRL + G", "(All)" },
    { "Gui/Settings", "CTRL + F", "(All)" },
    { "Toggle Camera Input", "CTRL + C", "(View_Mode)" },
    { "Save current file", "CTRL + S", "(Edit_Mode)" },
    { "Swap current line up/down", "ALT + Up/Down", "(Edit_Mode)" },
    { "Select text", "SHIFT + Up/Down/Left/Right", "(Edit_Mode)" },
    { "De-select region", "ESCAPE", "(Edit_Mode)" },
    { "Copy region to clipboard", "CTRL + C", "(Edit_Mode)" },
    { "Cut region to clipboard", "CTRL + D", "(Edit_Mode)" },
    { "Undo", "CTRL + Z", "(Edit_Mode)" },
    { "", "", "" },
    { "", "", "" },
};
   

void KeybindsTab::render() {

    size_t num_keybinds = sizeof(g_keybinds) / sizeof *g_keybinds;

    for(size_t i = 0; i < num_keybinds; i++) {
        ImGui::Text(g_keybinds[i][0]);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0, 0.5, 0.3, 1.0), g_keybinds[i][1]);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.3, 0.8, 0.5, 1.0), g_keybinds[i][2]);
    }

}


