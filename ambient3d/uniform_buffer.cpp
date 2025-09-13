#include <cstdio>

#include "uniform_buffer.hpp"
#include "external/glad.h"



void UniformBuffer::create(int binding_point, std::initializer_list<UBO_ELEMENT> element_sizes) {
    if(m_created) {
        fprintf(stderr, "ERROR! Trying to create uniform buffer, but its already created.\n");
        return;
    }
    
    m_total_sizeb = 0;
    m_num_elements = 0;

    for(const UBO_ELEMENT& n : element_sizes) {
        for(size_t i = 0; i < n.num; i++) {
            m_offset_map.push_back(m_total_sizeb);
            m_total_sizeb += n.elem_sizeb;
        }
        m_num_elements += n.num;
    }

    glGenBuffers(1, &this->id);
    glBindBuffer(GL_UNIFORM_BUFFER, this->id);
    glBufferData(GL_UNIFORM_BUFFER, m_total_sizeb, NULL, GL_STATIC_DRAW);

    glBindBufferRange(GL_UNIFORM_BUFFER, binding_point, this->id, 0, m_total_sizeb);

    m_binding_point = binding_point;
}

void UniformBuffer::update_element(int element_index, void* data, size_t size) {
    //printf("%s -> %li\n", __func__, m_offset_map[element_index]);
    glBindBuffer(GL_UNIFORM_BUFFER, this->id);
    glBufferSubData(GL_UNIFORM_BUFFER, m_offset_map[element_index], size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void UniformBuffer::free() {
    if(m_created && this->id > 0) {
        glDeleteBuffers(1, &this->id);
        m_created = false;
        this->id = 0;
    }
}



