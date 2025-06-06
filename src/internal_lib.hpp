#ifndef RAYMARCH_SANDBOX_INTERNAL_LIB_HPP
#define RAYMARCH_SANDBOX_INTERNAL_LIB_HPP

#include <string>
#include <list>


struct document_t {
    std::string code;
    std::string desc;
    std::string name;
    size_t num_newlines; // Used for rendering the text field correct size.
};

class InternalLib {
    public:
        static InternalLib& get_instance() {
            static InternalLib i;
            return i;
        }
        
        void create_source();
        void add_document(const char* code, const char* description);
        const std::string get_source();
        const char* get_vertex_src();

        std::list<struct document_t> documents;

        // Avoid accidental copies.
        InternalLib(InternalLib const&) = delete;
        void operator=(InternalLib const&) = delete;


    private:

        std::string source;
        InternalLib() {}
};




#endif
