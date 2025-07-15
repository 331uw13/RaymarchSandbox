#ifndef EDITOR_UNDO_HPP
#define EDITOR_UNDO_HPP

#include <vector>
#include <string>
#include <cstdint>

#define UNDOSTACK_LIMIT 100


struct UndoState {
    int64_t cursor_x;
    int64_t cursor_y;
    std::string data;
};

struct Cursor;

class UndoStack {

    public:
        UndoStack();


        void push_snapshot(const std::vector<std::string>* data_in, Cursor* cur_in);
        void pop_snapshot(std::vector<std::string>* data_out, Cursor* cur_out);
        void clear_snapshots();
        uint64_t latest_snapshot_hash();
    
    private:

        UndoState   m_stack [UNDOSTACK_LIMIT];
        size_t      m_stack_size;

};



#endif
