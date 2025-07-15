
#include "editor_undo.hpp"
#include "editor.hpp"
        

UndoStack::UndoStack() {
    this->clear_snapshots();
}


void UndoStack::push_snapshot(const std::vector<std::string>* data_in, Cursor* cursor_in) {
    if(m_stack_size+1 >= UNDOSTACK_LIMIT) {
        this->clear_snapshots();
    }

    UndoState* state = &m_stack[m_stack_size];
    state->cursor_x = cursor_in->x;
    state->cursor_y = cursor_in->y;
    state->data.clear();

    for(size_t i = 0; i < data_in->size(); i++) {
        state->data += (*data_in)[i] + '\n';
    }

    m_stack_size++;
}


void UndoStack::pop_snapshot(std::vector<std::string>* data_out, Cursor* cursor_out) {
    if(m_stack_size == 0) {
        return;
    }
    
    UndoState* state = &m_stack[m_stack_size-1];
    data_out->clear();

    cursor_out->x = state->cursor_x;
    cursor_out->y = state->cursor_y;
   
    size_t prev_newln_pos = 0;
    size_t num_lines = 0;

    for(size_t i = 0; i < state->data.size(); i++) {
        size_t newln = state->data.find('\n', prev_newln_pos);
        if(newln == std::string::npos) {
            break;
        } 
        //printf("\"%s\"\n", state->data.substr(prev_newln_pos, newln - prev_newln_pos).c_str());

        data_out->insert(
                data_out->begin() + num_lines, 
                state->data.substr(prev_newln_pos, newln - prev_newln_pos)
                );

        num_lines++;
        prev_newln_pos = newln+1;
    }

    m_stack_size--;
}

        
void UndoStack::clear_snapshots() {
    for(int i = 0; i < UNDOSTACK_LIMIT; i++) {
        m_stack[i] = (UndoState){ 0, 0, "" };
    }
    m_stack_size = 0;
}
        

uint64_t UndoStack::latest_snapshot_hash() {
    if(m_stack_size == 0) {
        return 0;
    }

    return std::hash<std::string>{}(m_stack[m_stack_size-1].data);
}



