#include "imgui_ext.hpp"

static constexpr ImVec4 DARK_BUTTON_BG  = ImVec4(0.2, 0.2, 0.2, 1.0);



bool ImGuiExt::DarkButton(const char* label, ImVec4 label_color) {

    ImVec4* colors = ImGui::GetStyle().Colors;

    ImVec4 oldcol_text = colors[ImGuiCol_Text];
    ImVec4 oldcol_button = colors[ImGuiCol_Button];
    ImVec4 oldcol_button_hovered = colors[ImGuiCol_ButtonHovered];
    ImVec4 oldcol_button_active = colors[ImGuiCol_ButtonActive];

    
    colors[ImGuiCol_Text] = label_color;
    colors[ImGuiCol_Button] = DARK_BUTTON_BG;
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.4, 0.4, 0.4, 1.0);
    colors[ImGuiCol_ButtonActive] = ImVec4(1.0, 1.0, 1.0, 1.0);


    bool retv = ImGui::Button(label);


    colors[ImGuiCol_Text] = oldcol_text;
    colors[ImGuiCol_Button] = oldcol_button;
    colors[ImGuiCol_ButtonHovered] = oldcol_button_hovered;
    colors[ImGuiCol_ButtonActive] = oldcol_button_active;

    return retv;
}



