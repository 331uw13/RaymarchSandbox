#ifndef FILEBROWSER_HPP
#define FILEBROWSER_HPP

#include <string>
#include <vector>

    
class RMSB;

enum FileType : int {
    REGULAR,
    DIRECTORY,
    SYMLINK,
    OTHER
};

struct File {
    std::string path; // Full path.
    std::string name; // File name.
    std::string extension;
    uint64_t sizeb;   // File size in bytes.
    FileType type;
};


namespace FileBrowserCallbacks {

    void shader_selected(RMSB* rmsb, const File& file);

};

// File browser should be first opened,
// then register callback to handle the event.
// FileBrowserCallbacks namespace could be used for it.
// When user clicks regular file it will call back.
// IMPORATNT NOTE: FileBrowser::open will clear the task callback.

class FileBrowser {
    public:

        std::vector<File> files;
        
        void open(
                const std::string  directory,
                const std::string& task_name,
                const std::string& favor_ext = "" // Favor arbitrary file extension to be first.
                );

        void close() { m_open = false; };
        bool is_open() { return m_open; };
        void register_task_callback(void(*callback)(RMSB*, const File&));

        void render(RMSB* rmsb);



        static FileBrowser& Instance() {
            static FileBrowser i;
            return i;
        }

    private:

        FileBrowser(){}

        std::string m_current_dir;
        std::string m_task_name;
        std::string m_favor_ext;

        void(*m_task_callback)(RMSB*, const File&);
        bool m_open;



};



#endif
