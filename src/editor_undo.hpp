#ifndef EDITOR_UNDO_HPP
#define EDITOR_UNDO_HPP

#include <string>
#include <vector>
#include <cstdint>

// TODO: Rename this file

inline constexpr auto MAX_UNDO_STEP_SIZE = 8;


// I have implement the UndoStack like this:
// Every time the editor recieves 
// character input or key input <backspace>, <enter> ..etc
// the UndoEvent is added to the UndoStack 'm_events' array.
// UndoCMD will tell what exactly happened in that point of time.
// 
// 'request_undo_step()' returns an array of pointers to events from the UndoStack
// 'save_step()' saves the current number of events for 'request_undo_step()'
//               to know how many events it must return for processing.
//
// NOTES:
//  - 'push_event' function will call 'save_event_steps' if
//    it has added 'MAX_UNDO_STEP_SIZE' amount of events



struct Cursor;
class  Editor;

enum UndoCMD {
    NONE,

    CHAR_ADDED,
    CHAR_REMOVED,

    STR_ADDED,
    STR_REMOVED,

    LINE_ADDED,
    LINE_REMOVED,

    LINE_SPLIT,

    STR_MOVED
};

struct UndoEvent {

    enum UndoCMD cmd;
    int64_t start_x;
    int64_t start_y;

    std::string data;


    void clear() {
        cmd = UndoCMD::NONE;
        data.clear(); 
        start_x = 0;
        start_y = 0;
    }

    UndoEvent() {
       clear();
    }

    UndoEvent(UndoCMD in_cmd, int64_t x, int64_t y, const std::string& in_data) {
        start_x = x;
        start_y = y;
        cmd = in_cmd;
        data = in_data;
    }
};


struct UndoStep {
    UndoEvent*  events [MAX_UNDO_STEP_SIZE];
    uint32_t    num_events;

    UndoStep() {
        for(uint32_t i = 0; i < MAX_UNDO_STEP_SIZE; i++) {
            events[i] = NULL;
        }
        num_events = 0;
    }
};

class UndoStack {

    public:
        UndoStack();
        ~UndoStack();

        void allocate(uint32_t num_max_events);
        void free_memory();


        void push_event(enum UndoCMD cmd, const Cursor& cur, const std::string& data);
        void push_event(enum UndoCMD cmd, const Cursor& cur, char data);
        void push_event(enum UndoCMD cmd, const Cursor& cur);

        void pop_event();
        void clear_events();
        UndoEvent* latest_event();// <- Returns NULL if there are no events.

        void save_step();
        UndoStep   request_undo_step(); 

    private:

        std::vector<uint32_t> m_step_indices;

        UndoEvent* m_events;
        uint32_t   m_num_events;
        uint32_t   m_num_max_events;
        uint32_t   m_num_saved_events;
        bool       m_mem_allocated;
};


#endif
