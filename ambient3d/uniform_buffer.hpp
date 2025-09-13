#ifndef AMBIENT3D_UNIFORM_BUFFER_HPP
#define AMBIENT3D_UNIFORM_BUFFER_HPP

#include <cstdint>
#include <vector>
#include <initializer_list>

// GLSL types size alligned example:
//  - int    : 4 bytes
//  - bool   : 4 bytes
//  - float  : 4 bytes
//  - vec3   : 16 bytes
//  - vec4   : 16 bytes
//  - mat4   : 64 bytes


struct UBO_ELEMENT {
    size_t num;        // How many elements.
    size_t elem_sizeb; // One element size in bytes.
};

class UniformBuffer {
    public:
        void create(int binding_point, std::initializer_list<UBO_ELEMENT> element_sizes);
        void update_element(int element_index, void* data, size_t size);
        void free();
        
        size_t size() { return m_num_elements; };
        size_t size_inbytes() { return m_total_sizeb; }
        
        int    get_binding_point() { return m_binding_point; }

        UniformBuffer() {
            m_created = false;
            this->id = 0;
        }

        uint32_t id;

    private:
        int               m_binding_point;
        bool              m_created;
        size_t            m_total_sizeb;
        size_t            m_num_elements;
        std::vector<int>  m_offset_map;
};




#endif
