#include "uniforms_tab.hpp"

#include "../shader_util.hpp"
#include "../rmsb.hpp"
#include "../imgui.h"


void UniformsTab::edit_uniform(struct uniform_t* uniform) {
    
    switch(uniform->type) {
        case UNIFORM_TYPE_COLOR:
            {
                ImGui::ColorPicker4("##UNIFORM_COLOR", uniform->values);
            }
            break;

        case UNIFORM_TYPE_VALUE:
            {
                ImGui::Text("TODO: Add adjustable min and max.");
                ImGui::SliderFloat("##UNIFORM_VALUE", &uniform->values[0], -1.0, 1.0, "%f");
            }
            break;

        case UNIFORM_TYPE_POSITION:
            {
                ImGui::Text("Not implemented yet.");
            }
            break;
    }

}



void UniformsTab::render(RMSB* rmsb) {
    // ----- Custom uniforms ------

    InternalLib& ilib = InternalLib::get_instance();
    
    static char name_buf[32] = { 0 };
    
    ImGui::Separator();

    ImGui::Text("Name:");
    ImGui::SameLine();
    ImGui::InputText("##UNIFORM_NAME_INPUT", name_buf, sizeof(name_buf)-1);
    Editor::get_instance().want_input = !ImGui::IsItemFocused();
    
    static int selected_index = 0;

    ImGui::Text("Type:");
    ImGui::SameLine();
    ImGui::Combo("##UNIFORM_FUNC_TYPE", &selected_index, 
            UNIFORM_TYPES_STR, NUM_UNIFORM_TYPES);

    ImGui::SameLine();
    if(ImGui::Button("Add")) {
        size_t name_size = strlen(name_buf);

        if(is_uniform_name_valid(name_buf, name_size)) {
           
            struct uniform_t uniform = (struct uniform_t) {
                .type = UNIFORM_TYPES[selected_index],
                .location = 0,
                .values = { 0, 0, 0, 1.0 },
                .name = name_buf,
            };

            ilib.add_uniform(&uniform);

            // Clear text input.
            memset(name_buf, 0, sizeof(name_buf));
        }
        else {
            rmsb->loginfo(ORANGE, "Uniform name is not valid.");
        }
    }

    ImGui::Separator();

    size_t id_counter = 0;

    std::list<struct uniform_t>::iterator uniform = ilib.uniforms.begin();
    while(uniform != ilib.uniforms.end()) {
        ImGui::PushID(id_counter);
       
        // Button for removing elements.
        if(ImGui::SmallButton("X")) {
            ilib.remove_uniform(&(*uniform));
            uniform = ilib.uniforms.erase(uniform);
            ImGui::PopID();
            continue;
        }

        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7, 0.6, 0.5, 1.0), uniform->name.c_str());
        ImGui::SameLine();

        if(ImGui::TreeNode(UNIFORM_TYPES_STR[uniform->type])) {
            edit_uniform(&(*uniform));
            ImGui::Separator();
            ImGui::TreePop();
        }

        ImGui::PopID();
        uniform++;
        id_counter++;
    }
}
