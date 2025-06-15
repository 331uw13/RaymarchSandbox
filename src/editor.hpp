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
    * Cross platform file IO.

*/

class Editor {
    public:
        static Editor& get_instance() {
            static Editor i;
            return i;
        }

        std::string title;

        void init();
        void quit();

        void render();
        void update();

        bool open;
        char char_input;

        Editor(Editor const&) = delete;
        void operator=(Editor const&) = delete;

        void clear();
        void load_data(const std::string& data);

        std::string get_data_str();

    // Settings:
        uint16_t page_size;
        float key_repeat_delay;
        float key_repeat_speed;
        int   opacity;

    private:

        std::map<std::string_view, int> m_color_map;

        std::string* get_line(int64_t y);
        void add_char(char c, int64_t x, int64_t y);
        void add_tabs(int64_t x, int64_t y, int count);
        void rem_char(int64_t x, int64_t y); // Remove character.
        int  count_begin_tabs(std::string* str); // Counts number of tabs until non-whitespace char is found.
        
        void move_cursor_to(int x, int y);
        void move_cursor(int xoff, int yoff);
        void clamp_cursor();

        Font font;

        void handle_key_input(int bypassed_check);
        void handle_char_inputs();
        void handle_frame_key_inputs();
        void handle_backspace();
        void handle_enter();

        std::vector<std::string> m_data;

        struct cursor_t {
            int64_t x;
            int64_t y;
        } cursor;

        Editor() {}

        int m_scroll;
        int m_fontsize;
        Vector2 m_charsize;
        Vector2 m_pos;
        Vector2 m_size;
        Vector2 m_grab_offset;
        int     m_margin;

        bool m_grab_offset_set;

        Color m_cursor_color;
        Color m_background_color;
        Color m_foreground_color;

        void update_charsize();

        // NOTE: For draw functions X, Y, width and height are multiplied with character size.
        void draw_rect(int x, int y, int w, int h, Color color);
        void draw_text(const char* text, float x, float y, Color color);
        void draw_text_glsl_syntax(const char* text, size_t text_size, float x, float y);
        Color dim_color(Color color, float t);
        
        void init_syntax_colors();

        float m_key_delay_timer;
        float m_key_repeat_timer;


};

#endif
