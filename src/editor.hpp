#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <string>
#include <vector>
#include <raylib.h>

// Very basic text editor for editting GLSL code.


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
        
        float key_repeat_delay;
        float key_repeat_speed;


    private:

        std::string* get_line(int64_t y);
        void add_char(char c, int64_t x, int64_t y);
        void move_cursor(int xoff, int yoff);
        void clamp_cursor();

        Font font;

        void handle_key_input(int bypassed_check);
        void handle_char_inputs();
        void handle_frame_key_inputs();

        std::vector<std::string> m_data;

        struct cursor_t {
            int64_t x;
            int64_t y;
        } cursor;

        Editor() {}

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
        Color dim_color(Color color, float t);
        
        float m_key_delay_timer;
        float m_key_repeat_timer;

};

#endif
