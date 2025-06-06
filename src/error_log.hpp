#ifndef ERROR_LOG_HPP
#define ERROR_LOG_HPP

#include <list>
#include <string>

#define ERRORLOG_WIDTH 600
#define ERRORLOG_HEIGHT 300


class ErrorLog {
    
    public:
        static ErrorLog& get_instance() {
            static ErrorLog i;
            return i;
        }

        ErrorLog() {}

        void add(const char* text);
        void clear();
        bool empty();
        void render();

        // Avoid accidental copies.
        ErrorLog(ErrorLog const&) = delete;
        void operator=(ErrorLog const&) = delete;

    private:

        std::list<std::string> m_log;

};



#endif
