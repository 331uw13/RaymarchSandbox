#include <raylib.h>
#include "imgui.h"


#include "error_log.hpp"
#include "rmsb_gui.hpp"
#include "internal_lib.hpp"



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
    ImGui::SetWindowFontScale(1.3);
    for(std::string line : m_log) {
    ImGui::TextWrapped(line.c_str());
    }

    ImGui::End();
}


void ErrorLog::get_error_position(int64_t* row, int64_t* col) {
    if(m_log.empty()) { 
        return;
    }

    /*
       ,- Row.
       v
    0:512(1): <the error message>
          ^
          `- Column.
    */
   
    const std::string& firstln = m_log.front();
    
    size_t colon_pos = firstln.find(':');
    if(colon_pos == std::string::npos) {
        fprintf(stderr, "%s: Missing ':' from error message.\n", __func__);
        return;
    }

    size_t left_bracet_pos = firstln.find('(');
    if(left_bracet_pos == std::string::npos) {
        fprintf(stderr, "%s: Missing '(' from error message.\n", __func__);
        return;
    }


    size_t right_bracet_pos = firstln.find(')');
    if(right_bracet_pos == std::string::npos) {
        fprintf(stderr, "%s: Missing ')' from error message.\n", __func__);
        return;
    }



    *row = atol(firstln.substr(colon_pos+1, left_bracet_pos - colon_pos - 1).c_str());
    *col = atol(firstln.substr(left_bracet_pos+1, right_bracet_pos - left_bracet_pos - 1).c_str());
}


