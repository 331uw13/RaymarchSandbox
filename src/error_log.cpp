#include <raylib.h>
#include "imgui.h"


#include "error_log.hpp"
#include "rmsb_gui.hpp"




void ErrorLog::add(const char* text) {
    m_log.push_back(text);
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



