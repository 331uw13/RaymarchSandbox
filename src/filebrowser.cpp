
#include <filesystem>
#include <algorithm>

#include "imgui.h"
#include "imgui_ext.hpp"
#include "filebrowser.hpp"
#include "rmsb_gui.hpp" 
#include "rmsb.hpp"


void FileBrowserCallbacks::shader_selected(RMSB* rmsb, const File& file) {
    Editor& editor = Editor::get_instance();

    if(editor.content_changed) {
        // ask_question() returns answer index.
        int answer = rmsb->gui.ask_question(
                "Current shader in editor is not saved.",
                { "Save and continue.", "Continue without saving!" });
        
        if(answer == 0) {
            editor.save(rmsb->shader_filepath.c_str());
        }
    }

    rmsb->shader_filepath = file.path;
    Editor::get_instance().load_file(file.path);
    rmsb->reload_state();
}


static constexpr ImVec4 FILE_COLORS[] = {
    ImVec4(0.7, 0.6, 0.4, 1.0),  // Regular.
    ImVec4(0.6, 0.6, 0.8, 1.0),  // Directory.
    ImVec4(0.3, 0.7, 0.7, 1.0),  // Symlink
    ImVec4(0.7, 0.5, 0.5, 1.0),  // Other...
};

void FileBrowser::open(const std::string  directory,
                       const std::string& task_name,
                       const std::string& favor_ext) {

    this->files.clear();
    m_task_callback = NULL;

    std::ranges::for_each(
        std::filesystem::directory_iterator{directory},
        [this](const auto& entry)
        {
            FileType type;
            uint64_t sizeb = 0;
            if(entry.is_regular_file()) {
                type = FileType::REGULAR;
                sizeb = entry.file_size();
            }
            else
            if(entry.is_directory()) {
                type = FileType::DIRECTORY;
            }
            else
            if(entry.is_symlink()) {
                type = FileType::SYMLINK;
            }
            else {
                type = FileType::OTHER;
            }
             
            this->files.push_back((File){
                entry.path(),
                std::filesystem::path(entry.path()).filename(),
                std::filesystem::path(entry.path()).extension(),
                sizeb,
                type
            });
        });

    // Sort files so directories start at first.
    // but most imporantly,
    // set files with 'favor_extension' to be first (if given)

    // IDEA: push element that must 
    // be in the beginning to back of array and
    // loop the array from back to front in render function

    // Keep track how many files with "favorite" extension.
    // used to know where directories are put.
    size_t num_favext = 0;
    int64_t size = this->files.size();

    for(int64_t i = size-1; i >= 0; i--) {
        const File* f = &this->files[i];

        if(!favor_ext.empty() 
        && f->extension == favor_ext) {
            this->files.push_back(*f);
            this->files.erase(this->files.begin()+i);
            num_favext++;
        }
        else
        if(f->type == FileType::DIRECTORY) {
            if(!this->files.empty()) {
                this->files.insert(this->files.end() - num_favext - 0, *f);
            }
            else {
                this->files.push_back(*f);
            }
            this->files.erase(this->files.begin()+i);
        }
    }

    m_favor_ext = favor_ext;
    m_current_dir = directory;
    m_task_name = task_name;
    m_open = true;
}

 
void FileBrowser::register_task_callback(void(*callback)(RMSB*, const File&)) {
    m_task_callback = callback;
    m_current_dir = std::filesystem::absolute(std::filesystem::current_path());
}
 
void FileBrowser::render(RMSB* rmsb) {
    if(!m_open) { return; }
    constexpr int      height = 300;
    constexpr ImVec4   filesize_color = ImVec4(0.5, 0.5, 0.5, 1.0);
    ImGui::SetNextWindowPos(ImVec2(GUI_WIDTH, 0));
    ImGui::SetNextWindowSize(ImVec2(FILEBROWSER_WIDTH, height));
    ImGui::Begin("FileBrowser", NULL, 
            ImGuiWindowFlags_NoMove
          | ImGuiWindowFlags_NoTitleBar
          | ImGuiWindowFlags_NoResize);

    if(ImGui::Button("Close")) {
        this->close();
    }
    ImGui::SameLine();
    if(ImGui::SmallButton("back")) {

        // Get parent directory of current directory.
        // for example: 
        // current = /home/user/something
        // parent  = /home/user
        std::string parent 
            = (std::filesystem::path { 
                    std::filesystem::absolute(m_current_dir)
            }).parent_path();

        if(parent.empty()) {
            parent = ".";
            rmsb->loginfo(RED, "Failed to get parent directory.");
        }

        // Save callback address because 'FileBrowser::open' will clear it.
        void(*callback)(RMSB*, const File&) = m_task_callback;
        this->open(parent, m_task_name, m_favor_ext);
        m_task_callback = callback;
    }

    ImGui::SameLine();
    ImGui::Text(m_task_name.c_str());
    ImGui::Separator();
    ImGui::BeginChild("##FILE_BROWSER_FILES", ImVec2(FILEBROWSER_WIDTH-15, height-50), true);

    const int64_t files_size = (int64_t)this->files.size();

    for(int64_t i = files_size-1; i >= 0; i--) {
        File& file = this->files[i];
        if(ImGuiExt::DarkButton(file.name.c_str(), FILE_COLORS[file.type])) {
           
            if(file.type == FileType::REGULAR) {
                if(!m_task_callback) {
                    rmsb->running = false;
                    fprintf(stderr, 
                            "[ERROR]: FileBrowser doesnt have registered callback.\n"
                            "         set it with FileBrowser::register_task_callback function\n"
                            "         after calling FileBrowser::open\n"
                            );
                    ImGui::EndChild();
                    ImGui::End();
                    return;
                }

                // Call registered task.
                m_task_callback(rmsb, file);
            }
            else
            if(file.type == FileType::DIRECTORY) {
                void(*callback)(RMSB*, const File&) = m_task_callback;
                this->open(file.path, m_task_name, m_favor_ext);
                m_task_callback = callback;
                break;
            }
        }
        if(file.type == FileType::REGULAR) {
            ImGui::SameLine();
            float kb = (float)file.sizeb/1000.0f;
            if(kb >= 1.0) {
                ImGui::TextColored(filesize_color, "%0.1f kB", kb);
            }
            else {
                ImGui::TextColored(filesize_color, "%li bytes", file.sizeb);
            }
        }
    }

    ImGui::EndChild();
    ImGui::End();
}


