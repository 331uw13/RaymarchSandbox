#include "uniforms_tab.hpp"

#include "../shader_util.hpp"
#include "../rmsb.hpp"
#include "../imgui.h"


void UniformsTab::edit_uniform(RMSB* rmsb, Uniform* uniform) {
    
    switch(uniform->type) {
        case UniformDataType::RGBA:
            {
                ImGui::ColorPicker4("##UNIFORM_COLOR", uniform->values);
            }
            break;

        case UniformDataType::SINGLE:
            {
                ImGui::Text("TODO: Add adjustable min and max.");
                ImGui::SliderFloat("##UNIFORM_VALUE", &uniform->values[0], 0.0, 1.0, "%f");
            }
            break;

        case UniformDataType::XYZ:
            {
                ImGui::TextColored(ImVec4(1.0, 0.5, 0.5, 1.0), "X=%0.3f", 
                        uniform->values[0]);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.5, 1.0, 0.5, 1.0), "Y=%0.3f", 
                        uniform->values[1]);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.5, 0.5, 1.0, 1.0), "Z=%0.3f", 
                        uniform->values[2]);
                if(ImGui::Button("Edit position")) {
                    rmsb->set_position_uniform_ptr(uniform);
                }
                ImGui::SameLine();
                if(ImGui::Button("done")) {
                    rmsb->set_position_uniform_ptr(NULL);
                }
            }
            break;


        case UniformDataType::INVALID:break;
        case UniformDataType::NUM_TYPES:break;
        default:break;
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
    //Editor::get_instance().want_input = !ImGui::IsItemFocused();
    
    static int selected_index = 0;

    ImGui::Text("Type:");
    ImGui::SameLine();
    ImGui::Combo("##UNIFORM_TYPE", &selected_index, 
            UNIFORM_DATA_TYPES_STR, UniformDataType::NUM_TYPES);

    ImGui::SameLine();
    if(ImGui::Button("Add")) {
        size_t name_size = strlen(name_buf);

        // Pre-check for invalid naming, this will avoid
        // removing the uniform and fixing the name if its bad for glsl.
        if(is_uniform_name_valid(name_buf, name_size)) {
           
            Uniform uniform = (Uniform) {
                .type = UNIFORM_DATA_TYPES[selected_index],
                .location = 0,
                .values = { 0, 0, 0, 1.0 },
                .name = name_buf,
            };

            uniform.name.push_back('\0');
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

    std::list<Uniform>::iterator uniform = ilib.uniforms.begin();
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

        if(ImGui::TreeNode(UNIFORM_DATA_TYPES_STR[uniform->type])) {
            edit_uniform(rmsb, &(*uniform));
            ImGui::Separator();
            ImGui::TreePop();
        }

        ImGui::PopID();
        uniform++;
        id_counter++;
    }
}
