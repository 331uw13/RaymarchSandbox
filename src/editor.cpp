#include <stdio.h>

#include "editor.hpp"


#define FONT_FILEPATH "./Px437_IBM_Model3x_Alt4.ttf"
#define FONT_SPACING 1.0
#define PADDING 3



static const int INPUT_KEYS[] = {
    KEY_LEFT,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN
};

#define IK_LEFT 0
#define IK_RIGHT 1
#define IK_UP 2
#define IK_DOWN 3



void Editor::init() {
    if(!FileExists(FONT_FILEPATH)) {
        fprintf(stderr, "\"%s\" Font file doesnt exist.\n",
                FONT_FILEPATH);
        return;
    }
    font = LoadFont(FONT_FILEPATH);

    m_background_color = (Color){ 20, 15, 12, 255 };
    m_foreground_color = (Color){ 230, 210, 150, 255 };
    m_cursor_color = (Color){ 80, 200, 100, 255 };

    m_fontsize = 16;
    m_size = (Vector2){ 450, 300 };
    m_pos = (Vector2){ GetScreenWidth()-(m_size.x+20), 40 };
    m_grab_offset = (Vector2){ 0, 0 };
    m_grab_offset_set = false;
    m_margin = 3;

    this->key_repeat_delay = 0.250;
    this->key_repeat_speed = 0.050;
    m_key_delay_timer = 0.0;
    m_key_repeat_timer = 0.0;

    for(int i = 0; i < 16; i++) {
        m_data.push_back("");
    }

    update_charsize();

    cursor.x = 0;
    cursor.y = 0;
}

void Editor::quit() {
    UnloadFont(font);
}


void Editor::render() {

    // Background.
    DrawRectangle(
            m_pos.x,
            m_pos.y,
            m_size.x,
            m_size.y,
            m_background_color);

    // Title bar.
    float titlebar_y = m_pos.y - m_charsize.y - PADDING;

    DrawRectangle(
            m_pos.x, titlebar_y, /* Position */ 
            m_size.x, m_charsize.y + PADDING, /* Size */  
            dim_color(m_background_color, 0.8)
            );

    draw_text(title.c_str(), 0, -1, m_foreground_color);

    // Cursor
    draw_rect(cursor.x + m_margin, cursor.y, 1, 1, m_cursor_color);


    // Data
    const size_t data_size = m_data.size();
    for(size_t i = 0; i < data_size; i++) {
        draw_text(m_data[i].c_str(), m_margin, i, m_foreground_color);
    }

}
        
void Editor::update_charsize() {

    int byte_count = 0;
    int codepoint = GetCodepointNext("#", &byte_count);
    int index = GetGlyphIndex(font, codepoint);

    float scale_factor = (float)m_fontsize / (float)font.baseSize;

    Rectangle rect = font.recs[index];
    m_charsize.x = rect.width * scale_factor + 2.0;
    m_charsize.y = m_fontsize;
}

void Editor::update() {
    Vector2 mouse = GetMousePosition();

    bool mouse_down = IsMouseButtonDown(MOUSE_LEFT_BUTTON);

    if((mouse.x > m_pos.x && mouse.x < m_pos.x + m_size.x)
    && (mouse.y > m_pos.y && mouse.y < m_pos.y + m_size.y)
    && mouse_down) {
        if(!m_grab_offset_set) {
            m_grab_offset_set = true;
            m_grab_offset.x = m_pos.x - mouse.x;
            m_grab_offset.y = m_pos.y - mouse.y;
        }

        m_pos.x = mouse.x + m_grab_offset.x;
        m_pos.y = mouse.y + m_grab_offset.y;


    }

    if(!mouse_down) {
        m_grab_offset_set = false;
    }

    handle_char_inputs();
    handle_frame_key_inputs();

}



// Private functions:

std::string* Editor::get_line(int64_t y) {
    const size_t data_size = m_data.size();
    if(y < 0) {
        y = 0;
    }
    else
    if(y >= (int64_t)data_size) {
        y = data_size - 1;
    }

    return &m_data[y];
}
        

void Editor::add_char(char c, int64_t x, int64_t y) {
    std::string* line = get_line(y);
    const size_t line_size = line->size();
    if(x < 0) {
        x = 0;
    }
    else
    if(x >= (int64_t)line_size) {
        line->push_back(c);
        return;
    }

    // Position is not at end of line
    line->insert((size_t)x, 1, c);
}

void Editor::handle_char_inputs() {

    char c = this->char_input;
    if(c == 0) {
        return;
    }

    if((c < 0x20) || (c > 0x7E)) {
        return;
    }

    add_char(c, cursor.x, cursor.y);
    cursor.x++;
    this->char_input = 0;
}

void Editor::clamp_cursor() {

    const size_t data_size = m_data.size();
    if(cursor.y < 0) {
        cursor.y = 0;
    }
    else
    if(cursor.y >= (int64_t)data_size) {
        cursor.y = data_size - 1;
    }

    std::string* line = get_line(cursor.y);

    if(cursor.x < 0) {
        cursor.x = 0;
    }
    else
    if(cursor.x > (int64_t)line->size()) {
        cursor.x = line->size();
    }
}

void Editor::move_cursor(int xoff, int yoff) {
    cursor.x += xoff;
    cursor.y += yoff;
    clamp_cursor();
}

void Editor::handle_key_input(int bypassed_check) {  
    bool(*check_key)(int) = (bypassed_check) ? IsKeyPressed : IsKeyDown;

    if(check_key(INPUT_KEYS[ IK_LEFT ])) {
        move_cursor(-1, 0);
    }
    if(check_key(INPUT_KEYS[ IK_RIGHT ])) {
        move_cursor(1, 0);
    }

}


void Editor::handle_frame_key_inputs() {

    const size_t num_keys = sizeof(INPUT_KEYS) / sizeof *INPUT_KEYS;

    bool any_key_down = false;
    bool allow_key_hold = false;
    bool allow_key_action = false;

    for(size_t i = 0; i < num_keys; i++) {
        if(IsKeyDown(INPUT_KEYS[i])) {
            any_key_down = true;
        }
    }

    if(!any_key_down) {
        m_key_repeat_timer = 0;
        m_key_delay_timer = 0;
        return;
    }
    

    m_key_delay_timer += GetFrameTime();
    allow_key_hold = (m_key_delay_timer > this->key_repeat_delay);
    
    if(!allow_key_hold) {
        handle_key_input(1);
        m_key_repeat_timer = 0;
        return;
    }

    m_key_repeat_timer += GetFrameTime();
    allow_key_action = (m_key_repeat_timer > this->key_repeat_speed);

    if(!allow_key_action) {
        return;
    }

    m_key_repeat_timer = 0;

    handle_key_input(0);
}


void Editor::draw_rect(int x, int y, int w, int h, Color color) {
    DrawRectangle(
            m_pos.x + x * m_charsize.x,
            m_pos.y + y * m_charsize.y,
            w * m_charsize.x,
            h * m_charsize.y,
            color
            );
}

void Editor::draw_text(const char* text, float x, float y, Color color) {    
    DrawTextEx(font, text, 
            (Vector2){
                m_pos.x + x * m_charsize.x,
                m_pos.y + y * m_charsize.y
            },
            m_fontsize, FONT_SPACING, color);
}

Color Editor::dim_color(Color color, float t) {
    return (Color){
        (unsigned char)((float)color.r * t),
        (unsigned char)((float)color.g * t),
        (unsigned char)((float)color.b * t),
        color.a
    };
}



