#include <raylib.h>
#include "imgui.h"


#include "error_log.hpp"
#include "rmsb_gui.hpp"




void ErrorLog::add(const char* text) {
  
    printf("%li\n", strlen(text));
    m_log.push_back(text);
    
    /*
    size_t text_size = strlen(text);
#define TEXT_BUF_SIZE 10000
    char buf[TEXT_BUF_SIZE] = { 0 };
    size_t buf_i = 0;

    for(size_t i = 0; i < text_size; i++) {
        if(text[i] == '\n') {
            m_log.push_back(buf);
            m_log.push_back("\0");
            buf_i = 0;
            memset(buf, 0, buf_i);
            continue;
        }

        buf[buf_i] = text[i];
        buf_i++;
    }
    */

}

void ErrorLog::clear() {
    m_log.clear();
}

bool ErrorLog::empty() {
    return m_log.empty();
}

void ErrorLog::render() {

    ImGui::SetNextWindowPos(ImVec2(0, GetScreenHeight() - ERRORLOG_HEIGHT));
    ImGui::SetNextWindowSize(ImVec2(ERRORLOG_WIDTH, ERRORLOG_HEIGHT));
    ImGui::Begin("Error Log", NULL,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    for(std::string line : m_log) {
        ImGui::TextWrapped(line.c_str());
    }

    ImGui::End();
}



