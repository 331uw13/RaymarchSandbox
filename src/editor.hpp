#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <string>
#include <vector>
#include <raylib.h>
#include <cstdint>
#include <map>

// Very basic text editor for editting GLSL code.

/*
    TODO LIST.

    * Callbacks for rendering so all kind of graphics libraries can use this.
    * Cross platform file IO (without raylib).
    * Make this class Not a signleton and create EditorManager to allow multiple editors.

*/

class RMSB;

class Editor {
    public:
        static Editor& get_instance() {
            static Editor i;
            return i;
        }

        std::string title;

        void init();
        void quit();

        void render(RMSB* rmsb);
        void update(RMSB* rmsb);

        bool open;
        char char_input;

        Editor(Editor const&) = delete;
        void operator=(Editor const&) = delete;

        void clear();
        void save(const std::string& filepath);
        void load_data(const std::string& data);

        std::string get_content();
        bool content_changed;

    // Settings:
        uint16_t page_size;
        float key_repeat_delay;
        float key_repeat_speed;
        int   opacity;
        
        // Difference check timer.
        // When 'm_diff_check_timer' reaches this variable's value.
        // The editor will compare its contents hash to previous one.
        float diff_check_delay;
        void reset_diff();
        void update_diff();

    private:

        double m_diff_check_timer;
        double m_idle_timer;

        struct selectreg_t {
            uint64_t start_x;
            uint64_t start_y;
            uint64_t end_x;
            uint64_t end_y;
            bool inverted_y;
            bool active;
        } m_select;

        
        std::map<std::string_view, int> m_color_map;
        Color get_keyword_color(char* buffer);
        Color get_selectbg_color(int y);

        std::string* get_line(int64_t y);
        void add_char(char c, int64_t x, int64_t y);
        void add_tabs(int64_t x, int64_t y, int count);
        void rem_char(int64_t x, int64_t y); // Remove character.
        int  count_begin_tabs(std::string* str); // Counts number of tabs until non-whitespace char is found.
        bool is_string_whitespace(std::string* str);

        void move_cursor_to(int64_t x, int64_t y);
        void move_cursor(int xoff, int yoff);
        void clamp_cursor();

        void swap_line(int64_t y, int offset);
        void remove_selected();

        Font font;

        void handle_key_input(int bypassed_check);
        void handle_char_inputs();
        void handle_frame_key_inputs();
        void handle_backspace();
        void handle_enter();
        void handle_select_with_mouse();
        void handle_select_with_keys();

        uint64_t m_content_hash;

        void get_selected(struct selectreg_t* reg);
        void start_selection();

        std::vector<std::string> m_data;

        // This variables value will keep track of "preferred column" for cursor.
        // When row is changed, it should keep the same X
        // but the empty rows has to be ignored.
        int64_t m_cursor_preferred_x;
        struct cursor_t {
            int64_t x;
            int64_t y;
        } cursor;


        struct cursor_t m_prev_cursor;

        Editor() {}

        int m_scroll;
        int m_fontsize;
        Vector2 m_charsize;
        Vector2 m_pos;
        Vector2 m_size;
        Vector2 m_grab_offset;
        int     m_margin;
        bool m_cursor_moved;
        bool m_grab_offset_set;

        // Colors.
        Color m_cursor_color;
        Color m_background_color;
        Color m_foreground_color;
        Color m_comment_color;
        Color m_selected_color;

        void update_charsize();

        // NOTE: For draw functions X, Y, width and height are multiplied with character size.
        void draw_rect(int x, int y, int w, int h, Color color);
        void draw_text(const char* text, float x, float y, Color color);
        void draw_text_glsl_syntax(const char* text, size_t text_size, float x, float y);
        void draw_selected_reg();

        Color dim_color(Color color, float t);

        void drawnclear_kwbuf(char* kwbuf, size_t* kwbuf_i, int* text_x, int text_y, Color color);

        void init_syntax_colors();

        float m_key_delay_timer;
        float m_key_repeat_timer;
        bool m_multiline_comment;
        std::string m_tmp_str; // Can be used by any private function.
};

#endif
