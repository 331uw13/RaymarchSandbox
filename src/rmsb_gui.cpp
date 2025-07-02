#include <raylib.h>
#include <stdio.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include "rmsb_gui.hpp"
#include "rmsb.hpp"
#include "shader_util.hpp"
#include "error_log.hpp"

#include "gui_tabs/uniforms_tab.hpp"
#include "gui_tabs/settings_tab.hpp"
#include "gui_tabs/editor_tab.hpp"
#include "gui_tabs/keybinds_tab.hpp"

void RMSBGui::init() {
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)GetWindowHandle(), true);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.DisplaySize = ImVec2(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT);
    
    //glfwSetCharCallback((GLFWwindow*)GetWindowHandle(), ImGui_ImplGlfw_CharCallback);
    ImGui::StyleColorsDark();

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text]                   = ImVec4(0.59f, 0.87f, 0.61f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 0.96f);
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

    colors[ImGuiCol_Text]                   = ImVec4(0.98f, 0.81f, 0.68f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.57f, 0.28f, 0.22f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.55f, 0.20f, 0.26f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.51f, 0.29f, 0.40f, 1.00f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.62f, 0.46f, 0.49f, 0.80f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.28f, 0.17f, 0.20f, 0.86f);
    colors[ImGuiCol_TabSelected]            = ImVec4(0.34f, 0.29f, 0.29f, 1.00f);

    this->view_functions = true;
    this->open = true;

}

void RMSBGui::quit() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void RMSBGui::update() {
    ImGuiIO& io = ImGui::GetIO();


    char input_char = GetCharPressed();
    if(input_char != 0) {
        Editor& editor = Editor::get_instance();
        if(editor.open && editor.has_focus) {
            editor.char_input = input_char;
        }
    }

    if(!this->open) {
        return;
    }

    Editor& editor = Editor::get_instance();


    if(input_char != 0) {
        io.AddInputCharacter(input_char);
    }

    if(editor.mouse_hovered) { 
        io.MousePos = ImVec2(GetScreenWidth()/2, GetScreenHeight()/2);
        return;
    }

    io.DisplaySize = ImVec2((float)GetScreenWidth(), (float)GetScreenHeight());

    Vector2 mouse = GetMousePosition();
    io.MousePos = ImVec2(mouse.x, mouse.y);

    io.MouseDown[0] = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    io.MouseDown[1] = IsMouseButtonDown(MOUSE_RIGHT_BUTTON);
    io.MouseDown[2] = IsMouseButtonDown(MOUSE_MIDDLE_BUTTON);

    io.MouseWheel += GetMouseWheelMove();


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
        int gui_height = GetScreenHeight();
        if(!ErrorLog::get_instance().empty()) {
            gui_height -= ERRORLOG_HEIGHT;
        }
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(GUI_WIDTH, gui_height));
        ImGui::Begin("Gui", NULL, 
              ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoResize);
        {


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
                if(ImGui::BeginTabItem("Keybinds")) {
                    KeybindsTab::render();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
       
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
    
    
        ImVec4* colors = ImGui::GetStyle().Colors;
        ImVec4 original_header_color = colors[ImGuiCol_Header];

        //colors[ImGuiCol_Header] = ImVec4(0.57f, 0.28f, 0.22f, 0.31f);

        for(struct document_t document : ilib.documents) {

            colors[ImGuiCol_Header] = ImVec4(
                    (float)document.color.red / 255.0,
                    (float)document.color.grn / 255.0,
                    (float)document.color.blu / 255.0,
                    1.0
                    );

            if(ImGui::CollapsingHeader(document.name.c_str())) {
                ImGui::PushID(counter);

                ImGui::Text(document.desc.c_str());
                if(!document.link.empty()) {
                    ImGui::TextLink(document.link.c_str());
                }
                if(ImGui::TreeNodeEx("Source", ImGuiTreeNodeFlags_DefaultOpen)) {

                    const float draw_height = (document.num_newlines+2) * ImGui::GetFontSize();
                    
                    ImGui::InputTextMultiline("##SOURCE", 
                            (char*)document.code.c_str()+'\0', document.code.size()+1,
                            ImVec2(FUNCTIONS_VIEW_WIDTH-50, draw_height), 
                            ImGuiInputTextFlags_ReadOnly);
                    ImGui::TreePop();
                }

                ImGui::PopID();
                ImGui::Separator();
            }
            counter++;
        }
        ImGui::End();
        
        colors[ImGuiCol_Header] = original_header_color;
    }


    ErrorLog& log = ErrorLog::get_instance();
    if(!log.empty()) {
        log.render();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
   

}
        

int RMSBGui::ask_question(const char* question, std::initializer_list<std::string_view> options) {
    int res = -1;

    EndDrawing(); // End current draw.
    float alpha = 0;


    Font font = Editor::get_instance().font;

    Vector2 center = (Vector2){
        (float)GetScreenWidth() / 2 - 100,
        (float)GetScreenHeight() / 2,
    };

    while(res < 0) {
        BeginDrawing();
        Vector2 mouse = GetMousePosition();
        
        if(alpha < 50.0) {
            alpha += GetFrameTime() * 300;
        }

        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 30, 17, 10, (uint8_t)alpha });

        DrawTextEx(font, question, center, 16, 1.0, (Color){ 255, 240, 180, 255 });
       
        Vector2 option_pos = center;
        option_pos.y += MeasureTextEx(font, question, 16, 1.0).y + 20;

        int counter = 0;

        for(std::string_view option_str : options) {

            Vector2 text_size = MeasureTextEx(font, option_str.data(), 16, 1.0);

            bool mouse_on_option
                = (mouse.x > option_pos.x-10 && mouse.x < option_pos.x+text_size.x+10)
                && (mouse.y > option_pos.y-2 && mouse.y < option_pos.y+text_size.y+2);

            DrawRectangle(
                    option_pos.x-10,
                    option_pos.y-2,
                    text_size.x+20,
                    text_size.y+4,
                    mouse_on_option ? (Color){ 70, 24, 20, 230 } : (Color){ 40, 20, 20, 200 }
                    );

            DrawTextEx(font, option_str.data(), option_pos, 16, 1.0, (Color){ 180, 180, 180, 255 });
            

            if(mouse_on_option && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                res = counter;
            }

            counter++;
            option_pos.x += text_size.x + 30;
        }
        EndDrawing();
    }

    BeginDrawing();
    return res;
}









