#include <raylib.h>
#include <stdio.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"


#include "rmsb_gui.hpp"
#include "rmsb.hpp"
#include "util.hpp"

#include "gui_tabs/uniforms_tab.hpp"
#include "gui_tabs/settings_tab.hpp"
#include "gui_tabs/editor_tab.hpp"


void RMSBGui::init() {
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)GetWindowHandle(), true);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.DisplaySize = ImVec2(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT);
    
    //glfwSetCharCallback((GLFWwindow*)GetWindowHandle(), ImGui_ImplGlfw_CharCallback);
    ImGui::StyleColorsDark();

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text]                   = ImVec4(0.59f, 0.87f, 0.61f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 0.86f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.24f, 0.37f, 0.91f, 0.67f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.21f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.17f, 0.94f, 0.32f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.97f, 0.38f, 0.64f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.59f, 0.19f, 0.40f, 0.58f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.26f, 0.12f, 0.19f, 1.00f);


    colors[ImGuiCol_FrameBg]                = ImVec4(0.37f, 0.21f, 0.29f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.25f, 0.14f, 0.17f, 0.83f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.18f, 0.08f, 0.14f, 0.67f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.43f, 0.16f, 0.29f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.27f, 0.44f, 0.32f, 1.00f);

    this->view_functions = false;
    this->open = true;
}

void RMSBGui::quit() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void RMSBGui::update() {
    if(!this->open) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2((float)GetScreenWidth(), (float)GetScreenHeight());

    Vector2 mouse = GetMousePosition();
    io.MousePos = ImVec2(mouse.x, mouse.y);

    io.MouseDown[0] = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    io.MouseDown[1] = IsMouseButtonDown(MOUSE_RIGHT_BUTTON);
    io.MouseDown[2] = IsMouseButtonDown(MOUSE_MIDDLE_BUTTON);

    io.MouseWheel += GetMouseWheelMove();

    char input_char = GetCharPressed();
    if(input_char != 0) {
        io.AddInputCharacter(input_char);
        Editor::get_instance().char_input = input_char;
    }

    // TODO: Add support for holding keys down.
    io.AddKeyEvent(ImGuiKey_Backspace, IsKeyPressed(KEY_BACKSPACE));
    io.AddKeyEvent(ImGuiKey_LeftArrow, IsKeyPressed(KEY_LEFT));
    io.AddKeyEvent(ImGuiKey_RightArrow, IsKeyPressed(KEY_RIGHT));
}   
    

void RMSBGui::render(RMSB* rmsb) {
    if(!this->open) {
        return;
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    //ImGui::ShowDemoWindow();

    if(this->open) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(GUI_WIDTH, GUI_HEIGHT));
        ImGui::Begin("Gui", NULL, 
              ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoResize);
        {
            //ImGui::ShowDemoWindow();


            if(ImGui::BeginTabBar("##SETTINGS_TAB")) {
                
                if(ImGui::BeginTabItem("General")) {
                    SettingsTab::render(rmsb);
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Uniform Input")) {
                    UniformsTab::render(rmsb);
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("GLSL Editor")) {
                    EditorSettingsTab::render(rmsb);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
       
            //SettingsTab::render(rmsb);
            //UniformsTab::render(rmsb);




            /*
            if(ImGui::TreeNode("Inputs")) {

                static char name_buf[32] = { 0 };

                ImGui::Text("Uniform name:");
                ImGui::SetKeyboardFocusHere();
                ImGui::InputText("##UNIFORM_NAME", name_buf, sizeof(name_buf)-1);
                ImGui::SameLine();
                if(ImGui::Button("Add uniform")) {
                    printf("Add '%s' | <type?>\n", name_buf);
                }

           
                ImGui::TreePop();
            }
            */


            /*
            ImGui::SeparatorText("Compile settings");
            // TODO: Auto compile.
            
            ImGui::SeparatorText("Custom uniforms");
            // TODO: Easily programmable timelines.
            
            ImGui::SeparatorText("Textures");
            // TODO: Easily programmable timelines.
            
            ImGui::SeparatorText("Camera settings");
            // TODO: 3D view toggle.

            ImGui::SeparatorText("Timeline settings");
            // TODO: Easily programmable timelines.
            */


        }
        ImGui::End();
    }
   
    if(this->view_functions) {
        InternalLib& ilib = InternalLib::get_instance();
        ImGui::SetNextWindowPos(ImVec2(GetScreenWidth()-FUNCTIONS_VIEW_WIDTH, 0));
        ImGui::SetNextWindowSize(ImVec2(FUNCTIONS_VIEW_WIDTH, GetScreenHeight()));
        ImGui::Begin("Functions", NULL, 
                  ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoTitleBar
                | ImGuiWindowFlags_NoResize
               );
        ImGui::SetWindowFontScale(1.0);
        int counter = 0;
        for(struct document_t document : ilib.documents) {
            if(ImGui::CollapsingHeader(document.name.c_str())) {
                ImGui::PushID(counter);
                ImGui::Text(document.desc.c_str());
                if(ImGui::TreeNode("Source")) {

                    const float draw_height = (document.num_newlines+2) * ImGui::GetFontSize();
                    ImGui::InputTextMultiline("##SOURCE", 
                            (char*)document.code.c_str()+'\0', document.code.size()+1,
                            ImVec2(FUNCTIONS_VIEW_WIDTH-50, draw_height), 
                            ImGuiInputTextFlags_ReadOnly);
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            counter++;
        }
        ImGui::End();
    }


    ErrorLog& log = ErrorLog::get_instance();
    if(!log.empty()) {
        log.render();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
   

}


