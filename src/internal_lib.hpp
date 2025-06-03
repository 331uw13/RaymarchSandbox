#ifndef RAYMARCH_SANDBOX_INTERNAL_LIB_HPP
#define RAYMARCH_SANDBOX_INTERNAL_LIB_HPP

#include <string>
#include <list>


struct func_t {
    std::string code;
    std::string desc;
    std::string name;
};

class InternalLib {
    public:
        static InternalLib& get_instance() {
            static InternalLib i;
            return i;
        }
        
        void create_source();
        void add_func(const char* code, const char* description);
        const std::string get_source();
        const char* get_vertex_src();

        std::list<struct func_t> functions;

    // Avoid accidental copies.
    InternalLib(InternalLib const&) = delete;
    void operator=(InternalLib const&) = delete;


    private:

        std::string source;
        InternalLib() {}
};




#endif
