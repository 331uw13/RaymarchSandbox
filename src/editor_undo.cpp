
#include <cstdlib>

#include "editor_undo.hpp"
#include "editor.hpp"


UndoStack::UndoStack() {
    m_events = NULL;
    m_num_events = 0;
    m_num_max_events = 0;
    m_mem_allocated = false;
    m_num_saved_events = 0;
}
 
UndoStack::~UndoStack() {
    this->free_memory();
}

void UndoStack::allocate(uint32_t num_max_events) {
    if(m_mem_allocated || (m_events != NULL)) {
        fprintf(stderr, "UndoStack has already allocated memory.\n");
        return;
    }

    m_events = new UndoEvent[num_max_events];
    if(!m_events) {
        fprintf(stderr, "Failed to allocate %li bytes of memory for undo stack.\n",
                num_max_events * sizeof(UndoEvent));
        return;
    }


    m_num_max_events = num_max_events;
    m_mem_allocated = true;
   
    this->clear_events();
}

void UndoStack::free_memory() {
    if(m_mem_allocated) {
        delete[] m_events;
        m_events = NULL;
        m_num_max_events = 0;
    }
}

void UndoStack::push_event(enum UndoCMD cmd, const Cursor& cur, const std::string& data) {
    printf("[%i] %s: \"%s\"\n", m_num_events, __func__, data.c_str());

    if(m_num_events+1 >= m_num_max_events) {
        this->clear_events(); // TODO: Maybe handle this somehow better?
    }

    m_events[m_num_events] = UndoEvent(cmd, cur.x, cur.y, data);
    m_num_events++;


    m_num_saved_events++;
    if(m_num_saved_events >= MAX_UNDO_STEP_SIZE) {
        this->save_step();
        m_num_saved_events = 0;
    }
}
        
void UndoStack::push_event(enum UndoCMD cmd, const Cursor& cur, char data) {
    // TODO: This should probably seriously be removed.
    std::string shit;
    shit.push_back(data);
    shit.push_back('\0');
    this->push_event(cmd, cur, shit);
}

void UndoStack::pop_event() {
    if(m_num_events == 0) {
        return;
    }

    m_num_events--;
}


/*
UndoEvent UndoStack::read_event() {
    return m_events[m_num_events];
}
*/

void UndoStack::clear_events() {
    if(!m_mem_allocated) {
        return;
    }
    
    for(uint32_t i = 0; i < m_num_max_events; i++) {
        m_events[i] = UndoEvent();
    }

    m_num_events = 0;
}




void UndoStack::save_step() {
    //m_step_indices.insert(m_step_indices.begin(), m_num_events);
    m_step_indices.push_back(m_num_events);
    printf("%s\n", __func__);
}

UndoStep UndoStack::request_undo_step() {
    UndoStep step = UndoStep();

    if(m_step_indices.empty() || (m_num_events == 0)) {
        return step;
    }

    int end = (int)m_step_indices.back();
    
    for(int i = m_num_events; i >= end; i--) {
        step.events[step.num_events] = &m_events[i];
        step.num_events++;
        if(step.num_events >= MAX_UNDO_STEP_SIZE) {
            break;
        }
    }

    m_step_indices.pop_back();
    if(m_step_indices.empty()) {
        m_step_indices.push_back(0);
    }

    return step;
}


