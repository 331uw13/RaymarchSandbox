#include <stdio.h>
#include <math.h>
#include <cstring>

#include "editor.hpp"
#include "rmsb.hpp"
#include "util.hpp"

#define FONT_FILEPATH "./Px437_IBM_Model3x_Alt4.ttf"
#define FONT_SPACING 1.0
#define PADDING 3
#define TAB_WIDTH 4

static const int INPUT_KEYS[] = {
    KEY_LEFT,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN,
    KEY_BACKSPACE,
    KEY_ENTER,
    KEY_TAB
};

#define IK_LEFT 0
#define IK_RIGHT 1
#define IK_UP 2
#define IK_DOWN 3
#define IK_BACKSPACE 4
#define IK_ENTER 5
#define IK_TAB 6


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
    m_comment_color = (Color){ 120, 120, 120, 255 };
    m_selected_color = (Color){ 25, 55, 30, 255 };
    
    this->has_focus = false;
    this->open = true;
    this->page_size = 40;
    this->opacity = 245;
    this->key_repeat_delay = 0.250;
    this->key_repeat_speed = 0.050;
    this->diff_check_delay = 1.0;
    
    m_clipboard.clear();
    m_fontsize = 16;
    m_size = (Vector2){ 700, (float)(this->page_size * m_fontsize) };
    m_pos = (Vector2){ GUI_WIDTH+20, 300 };
    m_grab_offset = (Vector2){ 0, 0 };
    m_grab_offset_set = false;
    m_margin = 3;
    m_scroll = 0;
    m_key_delay_timer = 0.0;
    m_key_repeat_timer = 0.0;
    m_multiline_comment = false;
    m_select.active = false;
    m_cursor_preferred_x = 0;

    this->init_syntax_colors();
    this->clear();
    update_charsize();

    cursor.x = 0;
    cursor.y = 0;
}

void Editor::quit() {
    UnloadFont(font);
}

void Editor::clear() {
    m_data.clear();
    //m_data.push_back("");
}

void Editor::load_data(const std::string& data) {
    std::string line = "";

    for(size_t i = 0; i < data.size(); i++) {
        if(data[i] == '\n') {
            m_data.push_back(line);
            line.clear();
            continue;
        }
        line += data[i];
    }

    reset_diff();
}

void Editor::save(const std::string& filepath) {
    SaveFileText(filepath.c_str(), (char*)(get_content().c_str()));
    reset_diff();
}

void Editor::update_diff() {
    this->content_changed = (m_content_hash != std::hash<std::string>{}(get_content()));
}

void Editor::reset_diff() {
    this->content_changed = false;
    m_content_hash = std::hash<std::string>{}(get_content());
}

std::string Editor::get_content() {
    std::string str = "";

    for(size_t i = 0; i < m_data.size(); i++) {
        str += m_data[i] + '\n';
    }

    return str;
}
        
        
Color Editor::get_selectbg_color(int y) {
    y = (y*30 + (int)sin(y)*5) % 360 + GetTime()*80;
    return ColorFromHSV((float)y, 0.5, 0.3);
}

void Editor::render(RMSB* rmsb) {
    if(!this->open) {
        return;
    }

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

    // Draw focus indicator
    DrawCircle(m_pos.x+m_charsize.x, m_pos.y-m_charsize.y/2, 3.5,
            this->has_focus ? (Color){ 30, 200, 30, 255 } : (Color){ 70, 70, 70, 255 });
    
    draw_text(title.c_str(), 2, -1, m_foreground_color);

    // Extra info for title bar.
    {
        if(rmsb->mode == EDIT_MODE) {
            int num_cols = m_size.x / m_charsize.x;
            draw_text("(Edit_Mode)", num_cols-12, -1, (Color){ 0x39, 0xA8, 0x44, 0xFF });
        }

        float ttext_x = (float)title.size() + 3;

        if(this->m_select.active) {
            draw_text("(Select)", ttext_x, -1, (Color){ 0x30, 0xB4, 0xD9, 0xFF });
            ttext_x += 9.0;
        }

        if(this->content_changed) {
            draw_text("(unsaved)", ttext_x, -1, (Color){ 80, 50, 50, 0xFF });
        }
    }


    
    size_t data_visible = (m_scroll + this->page_size);
    data_visible = (data_visible > m_data.size()) ? m_data.size() : data_visible;


    // Draw Selected region.
    draw_selected_reg();

    // Cursor
    DrawRectangleRounded(
            (Rectangle) {
                m_pos.x + (cursor.x + m_margin) * m_charsize.x,
                m_pos.y + (cursor.y - m_scroll) * m_charsize.y,
                m_charsize.x,
                m_charsize.y
            },
            0.6,
            4,
            m_cursor_color
            );

    // Draw Data
    int text_y = 0;
    m_multiline_comment = false;
    for(size_t i = m_scroll; i < data_visible; i++) {
        const std::string* line = &m_data[i];

        draw_text_glsl_syntax(line->c_str(), line->size(), m_margin, text_y);
        text_y++;
    }


    // Draw character at cursor position different color.
    // NOTE: m_cursor.color.a is the blinking effect.
    char tmp[2] = {
        (*get_line(cursor.y))[cursor.x],
        '\0'
    };
    draw_text(tmp, cursor.x + m_margin, cursor.y - m_scroll,
            (Color){ 20, (unsigned char)(200 - m_cursor_color.a/2), 20, 255 });

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

void Editor::start_selection() {
    m_select.active = true;
    m_select.start_x = cursor.x;
    m_select.start_y = cursor.y;

    if(m_prev_cursor.x > (int64_t)m_select.start_x) {
        m_select.start_x++;
    }
    else
    if(m_select.start_x > 0) {
        m_select.start_x--;
    }
    // Selection end position has to be updated when
    // the cursor moves. so this function will not handle it.
}

void Editor::handle_select_with_mouse() {
    if(m_cursor_moved && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if(!m_select.active) {
            start_selection();

            // When select mode is started with a mouse
            // 'm_cursor_moved' must be used
            // but it will only notice the current Y position.
            // and not where it actually should start so it must be fixed.

            if(m_prev_cursor.x == cursor.x && m_prev_cursor.y != cursor.y) {
                m_select.start_x = m_prev_cursor.x;
                m_select.start_y = m_prev_cursor.y;
            }
        }
    }

    // Selected region has to be updated.
    if(m_select.active) {
        m_select.end_x = cursor.x;
        m_select.end_y = cursor.y;
    }
    if(m_select.active && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        m_select.active = false;
    }
}

void Editor::handle_select_with_keys() {
    if(!m_select.active) {
        start_selection();
    }

    // Update selected region.
    m_select.end_x = cursor.x;
    m_select.end_y = cursor.y;
}

void Editor::update(RMSB* rmsb) {
    if(!this->open) {
        return;
    }
    Vector2 mouse = GetMousePosition();

    this->mouse_hovered = 
       (mouse.x > m_pos.x && mouse.x < m_pos.x + m_size.x)
    && (mouse.y > m_pos.y && mouse.y < m_pos.y + m_size.y);

    this->has_focus = this->mouse_hovered;

    if(this->mouse_hovered && IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
        if(!m_grab_offset_set) {
            m_grab_offset_set = true;
            m_grab_offset.x = m_pos.x - mouse.x;
            m_grab_offset.y = m_pos.y - mouse.y;
        }

        m_pos.x = mouse.x + m_grab_offset.x;
        m_pos.y = mouse.y + m_grab_offset.y;
    }
    else
    if(this->mouse_hovered && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        //this->has_focus = true;   
        cursor.x = (mouse.x - m_pos.x - (m_margin * m_charsize.x)) / m_charsize.x;
        cursor.y = (mouse.y - m_pos.y + (m_scroll * m_charsize.y)) / m_charsize.y; 
    }
    else
    if(!this->mouse_hovered && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        //this->has_focus = false;
    }


    if(this->has_focus) {
        float mouse_wheel = GetMouseWheelMove();
        if(mouse_wheel > 0) {
            move_cursor(0, -2);
        }
        else 
        if(mouse_wheel < 0) {
            move_cursor(0, 2);
        }
    }

    
    if(m_select.active && (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_C))) {
        copy_selected();
    }
    if(m_select.active && (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D))) {
        copy_selected();
        m_select.active = true;
        remove_selected();
    }
    if(!m_select.active && (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V))) {
        paste_clipboard();
    }


    if(!IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
        m_grab_offset_set = false;
    }

    if(!IsKeyDown(KEY_LEFT_CONTROL)) {
        handle_char_inputs();
        handle_frame_key_inputs();
    }

    m_background_color.a = this->opacity;

    // Cursor blink
    m_cursor_color.a = (unsigned char)(pow((sin(GetTime()*8)*0.5+0.5), 3.5)*200.0)+50;
    
    clamp_cursor();
    m_cursor_moved = (m_prev_cursor.x != cursor.x) || (m_prev_cursor.y != cursor.y);


    handle_select_with_mouse();

    m_prev_cursor = cursor;

    if(IsKeyPressed(KEY_ESCAPE)) {
        m_select.active = false;
    }


    // Update timers.

    const float frame_dt = GetFrameTime();
    m_idle_timer += frame_dt;
   
    if(rmsb->auto_reload
    && (m_idle_timer >= rmsb->auto_reload_delay)) {
        m_idle_timer = 0;
        if(content_changed) {
            //printf("RELOAD?\n");
            rmsb->reload_shader(USER_FALLBACK_OPTION);
            reset_diff();
        }
    }

    m_diff_check_timer += frame_dt;
    if(m_diff_check_timer >= this->diff_check_delay) {
        m_diff_check_timer = 0;
        update_diff();
    }
}

void Editor::get_selected(struct selectreg_t* reg) {
    *reg = m_select;

    // Start and end has to be flipped if start index doesnt make sense.

    reg->inverted_y = false;

    if(reg->end_y < reg->start_y) {
        int tmp_y = reg->start_y;
        reg->start_y = reg->end_y;
        reg->end_y = tmp_y;
       
        int tmp_x = reg->start_x;
        reg->start_x = reg->end_x;
        reg->end_x = tmp_x;
        reg->inverted_y = true;
    }
    else
    if(reg->start_x > reg->end_x
    && (reg->end_y == reg->start_y)) {
        int tmp_x = reg->start_x;
        reg->start_x = reg->end_x;
        reg->end_x = tmp_x;
    }
}


void Editor::copy_selected() {
    m_clipboard.clear();

    struct selectreg_t reg;
    get_selected(&reg);

    std::string* start_line = get_line(reg.start_y);
    std::string* end_line = get_line(reg.end_y);

    if(reg.start_y == reg.end_y) { /* Copy one line selection */
        m_clipboard = start_line->substr(reg.start_x, reg.end_x - reg.start_x);
    }
    else { /* Copy multiline selection */

        m_clipboard = start_line->substr(reg.start_x, start_line->size() - reg.start_x);
        
        m_clipboard.push_back('\n');
        for(size_t y = reg.start_y+1; y < reg.end_y; y++) {
            m_clipboard += *get_line(y) + '\n';
        }

        m_clipboard += end_line->substr(0, reg.end_x);
    }

    m_select.active = false;
}

void Editor::paste_clipboard() {
    std::string* current = get_line(cursor.y);
 
    // The first line length is needed to remove the "right substring"
    // If the clipboard content is a multiline and it was pasted in middle of some text.
    bool read_first_ln_size = true;
    bool move_rsubstr = false;
    int first_ln_size = 0;
  
    bool can_move_rsubstr = (cursor.x < (int64_t)current->size());
    const std::string rsubstr
        = can_move_rsubstr
        ? current->substr(cursor.x, current->size() - cursor.x) : "";
    

    int iy = 0;
    int ix = cursor.x;

    for(size_t i = 0; i < m_clipboard.size(); i++) {
        char c = m_clipboard[i];

        int64_t y = cursor.y + iy;

        if(c == '\n') {
            m_data.insert(m_data.begin() + y+1, "");
            iy++;
            ix = 0;

            move_rsubstr = true;
            read_first_ln_size = false;
            continue;
        }

        if(read_first_ln_size) {
            first_ln_size++;
        }
        add_char(m_clipboard[i], ix, y);
        ix++;
    }

    if(can_move_rsubstr && move_rsubstr) {
        current = get_line(cursor.y); // Update pointer after insert.

        const size_t erase_index = cursor.x + first_ln_size;
        if(erase_index >= current->size()) {
            fprintf(stderr, "%s: Trying to erase too much data.\n",
                    __func__);
            return;
        }
        current->erase(erase_index, current->size() - erase_index);
        
        // Now add the right substring back.
        m_data.insert(m_data.begin() + cursor.y+iy+1, rsubstr);

    }
}


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
        
void Editor::add_tabs(int64_t x, int64_t y, int count) {
    std::string* line = get_line(y);
    for(int i = 0; i < count; i++) {
        for(int i = 0; i < TAB_WIDTH; i++) {
            line->insert(x, " ");
        }
    }
}

void Editor::rem_char(int64_t x, int64_t y) {
    std::string* line = get_line(y);
    const size_t line_size = line->size();
    if(line_size == 0) {
        return;
    }
    if(x-1 < 0) {
        return;
    }

    if(x >= (int64_t)line_size) {
        line->pop_back();
        return;
    }

    line->erase((size_t)x-1, 1);
}
    
int Editor::count_begin_tabs(std::string* str) {
    int num_tabs = 0;
    int num_spaces = 0;

    for(size_t i = 0; i < str->size(); i++) {
        if((*str)[i] != 0x20) {
            break;
        }
        num_spaces++;
        if(num_spaces >= TAB_WIDTH) {
            num_spaces = 0;
            num_tabs++;
        }
    }

    return num_tabs;
}

void Editor::remove_selected() {
    if(!m_select.active) {
        return;
    }

    struct selectreg_t reg;
    get_selected(&reg);

    // First and last line has to be processed separately.
    std::string* first_line = get_line(reg.start_y);
    std::string* last_line = get_line(reg.end_y);

    if(reg.start_y == reg.end_y) { /* Remove one line selection */
        first_line->erase(reg.start_x, reg.end_x - reg.start_x);
        this->cursor.x = reg.start_x;
    }
    else { /* Remove multiline selection */

        if(reg.start_x < first_line->size()) {
            first_line->erase(reg.start_x, first_line->size() - reg.start_x);
        }

        const uint64_t lastsub_start = reg.end_x;
        const int64_t lastsub_end = last_line->size() - reg.end_x;

        if((lastsub_end > 0)
        && (lastsub_end < (int64_t)last_line->size())) {
            *first_line += last_line->substr(lastsub_start, lastsub_end);
        }

        // Delete lines in between
        // last_line will also get deleted from the array
        // if it will not have any data left.
        m_data.erase(
                m_data.begin() + reg.start_y + 1,
                m_data.begin() + reg.end_y + 1//(preserve_lastln ? 0 : 1)
                );

        this->cursor.x = reg.start_x;
        this->cursor.y = reg.start_y;
    }
        
    m_select.active = false;
    
}

void Editor::handle_backspace() {
    if(m_select.active) {
        remove_selected();
        return;
    }

    if(cursor.x > 0) {
        rem_char(cursor.x, cursor.y);
        move_cursor(-1, 0);
    }
    else
    if(cursor.y > 0) {
        cursor.x = 0;
        
        std::string* up = get_line(cursor.y-1);
        std::string* current = get_line(cursor.y);
        size_t current_size = current->size();

        *up += *current;

        m_data.erase(m_data.begin()+cursor.y);
        move_cursor(0, -1);
        move_cursor(up->size() - current_size, 0);
    }
}

void Editor::handle_enter() {
    if(cursor.x < 0) {
        cursor.x = 0;
    }
    
    std::string* current = get_line(cursor.y);
    size_t current_size = current->size();
    m_data.insert(m_data.begin()+cursor.y+1, ""); // Add new line.

    if(cursor.x < (int64_t)current_size) {
        std::string* below = get_line(cursor.y+1);
        
        *below += current->substr(cursor.x);
        current->erase(cursor.x, current->size());
        cursor.x = 0;
    }

    // Add tabs to the just added new line 
    // so it starts at the same column automatically.
    int num_btabs = count_begin_tabs(current);
    if(num_btabs > 0) {
        add_tabs(0, cursor.y+1, num_btabs);
    }

    cursor.x = num_btabs * TAB_WIDTH;
    move_cursor(0, 1);
}


void Editor::handle_char_inputs() {
    if(!this->has_focus) {
        return;
    }

    char c = this->char_input;
    if(c == 0) {
        return;
    }

    // Only printable ASCII characters.
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

bool Editor::is_string_whitespace(std::string* str) {
    bool ws = true;

    for(size_t i = 0; i < str->size(); i++) {
        if((*str)[i] != 0x20) {
            ws = false;
            break;
        }
    }

    return ws;
}

void Editor::move_cursor_to(int64_t x, int64_t y) {
   
    int max_vrow = this->page_size + m_scroll;
    int64_t y_diff = (y - cursor.y);
    int64_t x_diff = (x - cursor.x);

    if(
    (y >= max_vrow) /* Scroll up */
     ||
    ((y < m_scroll) && (m_scroll > 0))) /* Scroll down */
    {
        m_scroll += y_diff;
        if(m_scroll < 0) {
            m_scroll = 0;
        }
    }


    if((y_diff != 0) && (x_diff == 0)) {
        /* Line change, Update preferred column. */
        if(cursor.x > 0
        && !is_string_whitespace(get_line(cursor.y))
        && m_cursor_preferred_x < cursor.x
        ) {

            m_cursor_preferred_x = cursor.x;
        }
        x = m_cursor_preferred_x;
    }
    else
    if(y_diff == 0) {
        m_cursor_preferred_x = x;
    }

    cursor.x = x;
    cursor.y = y;
    clamp_cursor();
}

void Editor::move_cursor(int xoff, int yoff) {
    move_cursor_to(cursor.x + xoff, cursor.y + yoff);
}
        
void Editor::swap_line(int64_t y, int offset) {

    std::string* current = get_line(y);
    std::string* to = get_line(y + offset);
    m_tmp_str = *to;

    *to = *current;
    *current = m_tmp_str;
}

void Editor::handle_key_input(int bypassed_check) {  
    bool(*check_key)(int) = (bypassed_check) ? IsKeyPressed : IsKeyDown;

    if(check_key(INPUT_KEYS[ IK_LEFT ])) {
        move_cursor(-1, 0);
        if(IsKeyDown(KEY_LEFT_SHIFT)) {
            handle_select_with_keys();
        }
    }
    if(check_key(INPUT_KEYS[ IK_RIGHT ])) {
        move_cursor(1, 0);
        if(IsKeyDown(KEY_LEFT_SHIFT)) {
            handle_select_with_keys();
        }
    }
    if(check_key(INPUT_KEYS[ IK_DOWN ])) {
        if(IsKeyDown(KEY_LEFT_ALT)) {
            swap_line(cursor.y, 1);
        }
        else
        if(IsKeyDown(KEY_LEFT_SHIFT)) {
            handle_select_with_keys();
        }
        move_cursor(0, 1);
    }
    if(check_key(INPUT_KEYS[ IK_UP ])) {
        if(IsKeyDown(KEY_LEFT_ALT)) {
            swap_line(cursor.y, -1);
        }
        else
        if(IsKeyDown(KEY_LEFT_SHIFT)) {
            handle_select_with_keys();
        }
        move_cursor(0, -1);
    }
    

    if(check_key(INPUT_KEYS[ IK_BACKSPACE ])) {
        handle_backspace();
    }
    else
    if(check_key(INPUT_KEYS[ IK_ENTER ])) {
        handle_enter();
    }
    else
    if(check_key(INPUT_KEYS[ IK_TAB ])) {
        add_tabs(cursor.x, cursor.y, 1);
        move_cursor(TAB_WIDTH, 0);
    }

    m_idle_timer = 0;
}


void Editor::handle_frame_key_inputs() {
    if(!this->has_focus) {
        return;
    }

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


void Editor::draw_selected_reg() {
    if(m_select.active) {

        struct selectreg_t reg;
        get_selected(&reg);

        if(reg.start_y == reg.end_y) { /* Draw one line selection. */
            int width = reg.end_x - reg.start_x;
            int rect_y = reg.start_y - m_scroll;
            draw_rect(reg.start_x + m_margin, rect_y, width,
                    1, get_selectbg_color(rect_y));
        }
        else { /* Draw multiline selection */
            

            // Start Row. 
            int start_x_to_end = get_line(m_select.start_y)->size() - m_select.start_x;
            int start_row_y = m_select.start_y - m_scroll;
            if(start_row_y >= 0 && start_row_y < this->page_size) {
                draw_rect(
                        m_margin + (m_select.start_x * !reg.inverted_y),
                        start_row_y,
                        reg.inverted_y ? m_select.start_x : start_x_to_end,
                        1,
                        get_selectbg_color(start_row_y)
                        );
            }


            // End Row.
            int end_x_to_end = get_line(m_select.end_y)->size() - m_select.end_x;
            int end_row_y = m_select.end_y - m_scroll;
            draw_rect(
                    m_margin + (m_select.end_x * reg.inverted_y),
                    end_row_y,
                    reg.inverted_y ? end_x_to_end : m_select.end_x,
                    1,
                    get_selectbg_color(end_row_y)
                    );


            // Rows in between.
            for(uint64_t y = reg.start_y+1; y < reg.end_y; y++) {
                if(y >= (uint64_t)(m_scroll + this->page_size)) {
                    break;
                }
                else
                if(y < (uint64_t)m_scroll) {
                    continue;
                }
                size_t line_size = get_line(y)->size();
                draw_rect(m_margin, y - m_scroll, (line_size != 0) ? line_size : 1, 
                        1, get_selectbg_color(y - m_scroll));
            }
        }
    }
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

Color Editor::get_keyword_color(char* buffer) {
    Color color = m_foreground_color;

    std::map<std::string_view, int>::iterator elem = m_color_map.find(buffer);
    if(elem != m_color_map.end()) {
        color = GetColor(elem->second);
    }
    return color;
}

void Editor::drawnclear_kwbuf(char* kwbuf, size_t* kwbuf_i, int* text_x, int text_y, Color color) {
    draw_text(kwbuf, (float)*text_x, text_y, color);
    *text_x += *kwbuf_i;
    memset(kwbuf, 0, *kwbuf_i);
    *kwbuf_i = 0;
}


void Editor::draw_text_glsl_syntax(const char* text, size_t text_size, float x, float y) {

    #define KEYWORD_BUFFER_SIZE 32

    char kwbuf[KEYWORD_BUFFER_SIZE+1] = { 0 };
    size_t kwbuf_i = 0;
    int text_x = x;

    bool is_comment = false;

    for(size_t i = 0; i < text_size; i++) {
        const char c = text[i];
        const bool line_end = (i+1 >= text_size);

        // There are few checks for "special" character <-- (in the sense of special for keyword)
        // That is because for example: "if(something)" or "return;" is valid but
        // if we only detect that keyword
        // ends at "space", end of line or when 
        // keyword buffer would grow too big.
        // that doesnt get highlighted.

        const bool special = (c == '(') || (c == ';') || (c == '.');

        // Check for comments.
        if(!line_end) {
            const char next_char = text[i+1];
            if(c == '/' && next_char == '/') { /* Normal comment started */
                // Draw the current data in kwbuf.
                // If there are no spaces between "example//comment"
                // it will show "example" in comment color.
                drawnclear_kwbuf(kwbuf, &kwbuf_i, &text_x, (int)y, get_keyword_color(kwbuf));
                is_comment = true;
            }
            else
            if(c == '/' && next_char == '*') { /* MultiLine comment started */
                drawnclear_kwbuf(kwbuf, &kwbuf_i, &text_x, (int)y, get_keyword_color(kwbuf));
                m_multiline_comment = true;
            }
            else
            if(c == '*' && next_char == '/') { /* MultiLine comment ended */
                kwbuf[kwbuf_i++] = '*';
                kwbuf[kwbuf_i++] = '/';
                drawnclear_kwbuf(kwbuf, &kwbuf_i, &text_x, (int)y, m_comment_color);
                m_multiline_comment = false;
                i++;
                continue;
            }
        }

        const bool comment_enabled = (is_comment || m_multiline_comment);

        if(
           (c == 0x20)
        || special
        || (line_end)
        || (kwbuf_i+1 >= KEYWORD_BUFFER_SIZE)
        
        ){
            bool pre_added = false;
            if(c != 0x20 && !special) {
                // Character is not "space":
                // Add it before searching the color from hashmap
                // The last character may be something that doesnt belong to keyword.
                // Also no space characters are wanted in hashmap
                kwbuf[kwbuf_i++] = c;
                pre_added = true;
            }

            Color color = !comment_enabled ? get_keyword_color(kwbuf) : m_comment_color;

            if(!pre_added && !special) {
                // Character was space. Add it after searching for keyword color
                // to render the text correctly.
                kwbuf[kwbuf_i++] = c;
            }

            draw_text(kwbuf, text_x, y, color);
            text_x += kwbuf_i;
            
            if(special) {
                const char tmp[2] = { c, '\0' };
                draw_text(tmp, text_x, y, !comment_enabled ? m_foreground_color : m_comment_color);
                text_x += 1;
            }

            memset(kwbuf, 0, kwbuf_i);
            kwbuf_i = 0;
            continue;
        }

        if(line_end) {
            is_comment = false;
        }
        
        kwbuf[kwbuf_i++] = c;
    }
}

void Editor::init_syntax_colors() {
    m_color_map.clear();

    // RGBA
    
    const int TYPE = 0x9F5255FF;
    const int STATEMENT = 0x68E681FF;
    const int GLSL_FUNCTION = 0xED9632FF;
    const int INTERNAL = 0xA0C242FF;
    const int INTERNAL_TYPE = 0xBF8C56FF;
    const int GLOBAL = 0x2C9633FF;

    m_color_map["Material"] = INTERNAL_TYPE;

    m_color_map["SphereSDF"] = INTERNAL;
    m_color_map["BoxSDF"] = INTERNAL;
    m_color_map["TorusSDF"] = INTERNAL;
    m_color_map["BoxFrameSDF"] = INTERNAL;
    
    m_color_map["Mdiffuse"] = INTERNAL;
    m_color_map["Mspecular"] = INTERNAL;
    m_color_map["Mdistance"] = INTERNAL;
    m_color_map["Mshine"] = INTERNAL;
    m_color_map["ColorRGB"] = INTERNAL;
    m_color_map["Camera"] = GLOBAL;
    m_color_map["Ray"] = GLOBAL;
    m_color_map["CameraInputRotation"] = INTERNAL;
    m_color_map["Noise"] = INTERNAL;
    m_color_map["RayDir"] = INTERNAL;
    m_color_map["Raymarch"] = INTERNAL;
    m_color_map["ComputeNormal"] = INTERNAL;
    m_color_map["ComputeLight"] = INTERNAL;
    m_color_map["RepeatINF"] = INTERNAL;
    m_color_map["RepeatLIM"] = INTERNAL;
    m_color_map["RotateM3"] = INTERNAL;
    m_color_map["RotateM2"] = INTERNAL;
    m_color_map["Palette"] = INTERNAL;
    m_color_map["ApplyFog"] = INTERNAL;
    m_color_map["MaterialMin"] = INTERNAL;
    m_color_map["MaterialMax"] = INTERNAL;
    m_color_map["MixMaterial"] = INTERNAL;
    m_color_map["BlendMaterials"] = INTERNAL;
    m_color_map["SmoothVoronoi2D"] = INTERNAL;
    m_color_map["SmoothVoronoi3D"] = INTERNAL;
    m_color_map["Hash2"] = INTERNAL;
    m_color_map["Hash3"] = INTERNAL;

    m_color_map["="] = 0xD48646FF;
    m_color_map["=="] = 0xD48646FF;
    m_color_map["struct"] = 0xC537DBFF;

    m_color_map["radians"] = GLSL_FUNCTION;
    m_color_map["degrees"] = GLSL_FUNCTION;
    m_color_map["sin"] = GLSL_FUNCTION;
    m_color_map["cos"] = GLSL_FUNCTION;
    m_color_map["tan"] = GLSL_FUNCTION;
    m_color_map["asin"] = GLSL_FUNCTION;
    m_color_map["acos"] = GLSL_FUNCTION;
    m_color_map["atan"] = GLSL_FUNCTION;
    m_color_map["sinh"] = GLSL_FUNCTION;
    m_color_map["cosh"] = GLSL_FUNCTION;
    m_color_map["tanh"] = GLSL_FUNCTION;
    m_color_map["asinh"] = GLSL_FUNCTION;
    m_color_map["acosh"] = GLSL_FUNCTION;
    m_color_map["atanh"] = GLSL_FUNCTION;
    m_color_map["pow"] = GLSL_FUNCTION;
    m_color_map["exp"] = GLSL_FUNCTION;
    m_color_map["log"] = GLSL_FUNCTION;
    m_color_map["exp2"] = GLSL_FUNCTION;
    m_color_map["log2"] = GLSL_FUNCTION;
    m_color_map["sqrt"] = GLSL_FUNCTION;
    m_color_map["inversesqrt"] = GLSL_FUNCTION;
    m_color_map["abs"] = GLSL_FUNCTION;
    m_color_map["sign"] = GLSL_FUNCTION;
    m_color_map["floor"] = GLSL_FUNCTION;
    m_color_map["trunc"] = GLSL_FUNCTION;
    m_color_map["round"] = GLSL_FUNCTION;
    m_color_map["roundEven"] = GLSL_FUNCTION;
    m_color_map["ceil"] = GLSL_FUNCTION;
    m_color_map["fract"] = GLSL_FUNCTION;
    m_color_map["mod"] = GLSL_FUNCTION;
    m_color_map["modf"] = GLSL_FUNCTION;
    m_color_map["min"] = GLSL_FUNCTION;
    m_color_map["max"] = GLSL_FUNCTION;
    m_color_map["clamp"] = GLSL_FUNCTION;
    m_color_map["mix"] = GLSL_FUNCTION;
    m_color_map["step"] = GLSL_FUNCTION;
    m_color_map["smoothstep"] = GLSL_FUNCTION;
    m_color_map["isnan"] = GLSL_FUNCTION;
    m_color_map["isinf"] = GLSL_FUNCTION;
    m_color_map["floatBitToInt"] = GLSL_FUNCTION;
    m_color_map["intBitsToFloat"] = GLSL_FUNCTION;
    m_color_map["fma"] = GLSL_FUNCTION;
    m_color_map["frexp"] = GLSL_FUNCTION;
    m_color_map["ldexp"] = GLSL_FUNCTION;
    m_color_map["length"] = GLSL_FUNCTION;
    m_color_map["distance"] = GLSL_FUNCTION;
    m_color_map["dot"] = GLSL_FUNCTION;
    m_color_map["cross"] = GLSL_FUNCTION;
    m_color_map["normalize"] = GLSL_FUNCTION;
    m_color_map["faceforward"] = GLSL_FUNCTION;
    m_color_map["reflect"] = GLSL_FUNCTION;
    m_color_map["refract"] = GLSL_FUNCTION;
    m_color_map["matrixCompMult"] = GLSL_FUNCTION;
    m_color_map["outerProduct"] = GLSL_FUNCTION;
    m_color_map["transpose"] = GLSL_FUNCTION;
    m_color_map["determinant"] = GLSL_FUNCTION;
    m_color_map["inverse"] = GLSL_FUNCTION;
    m_color_map["lessThan"] = GLSL_FUNCTION;
    m_color_map["lessThanEqual"] = GLSL_FUNCTION;
    m_color_map["greaterThan"] = GLSL_FUNCTION;
    m_color_map["greaterThanEqual"] = GLSL_FUNCTION;
    m_color_map["equal"] = GLSL_FUNCTION;
    m_color_map["notEqual"] = GLSL_FUNCTION;
    m_color_map["any"] = GLSL_FUNCTION;
    m_color_map["all"] = GLSL_FUNCTION;
    m_color_map["not"] = GLSL_FUNCTION;
    m_color_map["textureSize"] = GLSL_FUNCTION;
    m_color_map["textureQueryLod"] = GLSL_FUNCTION;
    m_color_map["texture"] = GLSL_FUNCTION;
    m_color_map["textureGrad"] = GLSL_FUNCTION;
    m_color_map["texture1D"] = GLSL_FUNCTION;
    m_color_map["texture2D"] = GLSL_FUNCTION;
    m_color_map["texture3D"] = GLSL_FUNCTION;
    m_color_map["textureCube"] = GLSL_FUNCTION;
    m_color_map["shadow1D"] = GLSL_FUNCTION;
    m_color_map["shadow2D"] = GLSL_FUNCTION;
    m_color_map["shadow3D"] = GLSL_FUNCTION;
    m_color_map["dFdx"] = GLSL_FUNCTION;
    m_color_map["dFdy"] = GLSL_FUNCTION;
    m_color_map["fwidth"] = GLSL_FUNCTION;
    m_color_map["noise1"] = GLSL_FUNCTION;
    m_color_map["noise2"] = GLSL_FUNCTION;
    m_color_map["noise3"] = GLSL_FUNCTION;
    m_color_map["noise4"] = GLSL_FUNCTION;

    m_color_map["if"] = STATEMENT;
    m_color_map["else"] = STATEMENT;
    m_color_map["while"] = STATEMENT;
    m_color_map["for"] = STATEMENT;
    m_color_map["switch"] = STATEMENT;
    m_color_map["case"] = STATEMENT;
    m_color_map["default"] = STATEMENT;
    m_color_map["break"] = STATEMENT;
    m_color_map["continue"] = STATEMENT;
    m_color_map["return"] = STATEMENT;

    m_color_map["uint"] = TYPE;
    m_color_map["int"] = TYPE;
    m_color_map["float"] = TYPE;
    m_color_map["double"] = TYPE;
    m_color_map["void"] = TYPE;
    m_color_map["bool"] = TYPE;
    m_color_map["vec2"] = TYPE;
    m_color_map["vec3"] = TYPE;
    m_color_map["vec4"] = TYPE;
    m_color_map["ivec2"] = TYPE;
    m_color_map["ivec3"] = TYPE;
    m_color_map["ivec4"] = TYPE;
    m_color_map["dvec2"] = TYPE;
    m_color_map["dvec3"] = TYPE;
    m_color_map["dvec4"] = TYPE;
    m_color_map["uvec2"] = TYPE;
    m_color_map["uvec3"] = TYPE;
    m_color_map["uvec4"] = TYPE;
    m_color_map["mat2"] = TYPE;
    m_color_map["mat3"] = TYPE;
    m_color_map["mat4"] = TYPE;
    m_color_map["dmat2"] = TYPE;
    m_color_map["dmat3"] = TYPE;
    m_color_map["dmat4"] = TYPE;
    m_color_map["mat2x2"] = TYPE;
    m_color_map["mat2x3"] = TYPE;
    m_color_map["mat2x4"] = TYPE;
    m_color_map["mat3x2"] = TYPE;
    m_color_map["mat3x3"] = TYPE;
    m_color_map["mat3x4"] = TYPE;
    m_color_map["dmat3x2"] = TYPE;
    m_color_map["dmat3x3"] = TYPE;
    m_color_map["dmat3x4"] = TYPE;
    m_color_map["mat4x2"] = TYPE;
    m_color_map["mat4x3"] = TYPE;
    m_color_map["mat4x4"] = TYPE;
    m_color_map["dmat4x2"] = TYPE;
    m_color_map["dmat4x3"] = TYPE;
    m_color_map["dmat4x4"] = TYPE;
    
   

}

Color Editor::dim_color(Color color, float t) {
    return (Color){
        (unsigned char)((float)color.r * t),
        (unsigned char)((float)color.g * t),
        (unsigned char)((float)color.b * t),
        color.a
    };
}


