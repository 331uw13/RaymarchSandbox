#ifndef LOG_FILE_HPP
#define LOG_FILE_HPP



enum LogfileInfoType {
    INFO = 0,
    WARNING,
    ERROR,
    FATAL
};


#define append_logfile(infotype, info_fmt, ...)\
    append_logfile_ext(infotype, __func__, __FILE__, info_fmt, ##__VA_ARGS__)

void append_logfile_ext(
        enum LogfileInfoType itype,
        const char* caller_func,
        const char* caller_func_file,
        const char* info_fmt, ...
        );

void assign_logfile(const char* filepath);
void close_logfile();





#endif
