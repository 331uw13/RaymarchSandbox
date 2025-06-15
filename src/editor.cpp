#include <stdio.h>
#include <math.h>
#include <cstring>

#include "editor.hpp"


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

    this->page_size = 40;
    this->opacity = 200;
    this->key_repeat_delay = 0.250;
    this->key_repeat_speed = 0.050;
    
    m_fontsize = 16;
    m_size = (Vector2){ 700, (float)(this->page_size * m_fontsize) };
    m_pos = (Vector2){ GetScreenWidth()-(m_size.x+20), 40 };
    m_grab_offset = (Vector2){ 0, 0 };
    m_grab_offset_set = false;
    m_margin = 3;
    m_scroll = 0;

    m_key_delay_timer = 0.0;
    m_key_repeat_timer = 0.0;

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
    m_data.push_back("");
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
}
        
std::string Editor::get_data_str() {
    std::string str = "";

    for(size_t i = 0; i < m_data.size(); i++) {
        str += m_data[i] + '\n';
    }

    return str;
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

    draw_rect(cursor.x + m_margin, cursor.y - m_scroll, 1, 1, m_cursor_color);

    /*
    DrawRectangle(
            m_pos.x + (m_margin + cursor.x) * m_charsize.x,
            m_pos.y + cursor.y * m_charsize.y + (m_charsize.y/1.5),
            m_charsize.x,
            m_charsize.y / 4,
            m_cursor_color
           );
    */

    // Data
    size_t data_visible = (m_scroll + this->page_size);
    data_visible = (data_visible > m_data.size()) ? m_data.size() : data_visible;
    int y = 0;
    for(size_t i = m_scroll; i < data_visible; i++) {
        const std::string* line = &m_data[i];
        draw_text_glsl_syntax(line->c_str(), line->size(), m_margin, y);
        //draw_text(m_data[i].c_str(), m_margin, y, m_foreground_color);
        y++;
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

    m_background_color.a = this->opacity;
    m_cursor_color.a = (unsigned char)((sin(GetTime()*8)*0.5+0.5)*200.0)+50;

    clamp_cursor();
}



// Private functions:
    
void render_syntax_highlight() {
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

void Editor::handle_backspace() {
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
    std::string* current = get_line(cursor.y);
    size_t current_size = current->size();

    m_data.insert(m_data.begin()+cursor.y+1, "");

    if(cursor.x < (int64_t)current_size) {
        std::string* below = get_line(cursor.y+1);

        *below += current->substr(cursor.x);
        current->erase(cursor.x, current->size());
        cursor.x = 0;
    }

    int num_btabs = count_begin_tabs(current);
    if(num_btabs > 0) {
        add_tabs(0, cursor.y+1, num_btabs);
    }

    cursor.x = num_btabs * TAB_WIDTH;
    move_cursor(0, 1);
}


void Editor::handle_char_inputs() {

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


void Editor::move_cursor_to(int x, int y) {
   
    int max_vrow = this->page_size + m_scroll;

    // FIXME: Scroll may be "left behind" if Y was set to offscreen.
    if(y >= max_vrow) {
        m_scroll++;
    }
    else
    if((y < m_scroll) && (m_scroll > 0)) {
        m_scroll--;
    }

    cursor.x = x;
    cursor.y = y;
    clamp_cursor();

}

void Editor::move_cursor(int xoff, int yoff) {
    move_cursor_to(cursor.x + xoff, cursor.y + yoff);
}

void Editor::handle_key_input(int bypassed_check) {  
    bool(*check_key)(int) = (bypassed_check) ? IsKeyPressed : IsKeyDown;

    if(check_key(INPUT_KEYS[ IK_LEFT ])) {
        move_cursor(-1, 0);
    }
    if(check_key(INPUT_KEYS[ IK_RIGHT ])) {
        move_cursor(1, 0);
    }
    if(check_key(INPUT_KEYS[ IK_DOWN ])) {
        move_cursor(0, 1);
    }
    if(check_key(INPUT_KEYS[ IK_UP ])) {
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
        
void Editor::draw_text_glsl_syntax(const char* text, size_t text_size, float x, float y) {

    #define KEYWORD_BUFFER_SIZE 32

    char kwbuf[KEYWORD_BUFFER_SIZE+1] = { 0 };
    size_t kwbuf_i = 0;
    float text_x = x;

    for(size_t i = 0; i < text_size; i++) {
        char c = text[i];
        const bool line_end = (i+1 >= text_size);
       
        // There are few checks for "special" character <-- (in the sense of special for keyword)
        // That is because for example: "if(something)" or "return;" is valid
        // but if we only detect that keyword
        // ends at "space", end of line or when 
        // keyword buffer would grow too big.
        // that doesnt get highlighted.

        bool special = (c == '(') || (c == ';') || (c == '.');

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

            Color color = m_foreground_color;

            std::map<std::string_view, int>::iterator elem = m_color_map.find(kwbuf);
            if(elem != m_color_map.end()) {
                color = GetColor(elem->second);
            }


            if(!pre_added && !special) {
                // Character was space. Add it after searching for keyword color
                // to render the text correctly.
                kwbuf[kwbuf_i++] = c;
            }

            draw_text(kwbuf, text_x, y, color);
            text_x += kwbuf_i;
            
            if(special) {
                const char tmp[2] = { c, '\0' };
                draw_text(tmp, text_x, y, m_foreground_color);
                text_x += 1;
            }

            memset(kwbuf, 0, kwbuf_i);
            kwbuf_i = 0;
            continue;
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

    m_color_map["Mdiffuse"] = INTERNAL;
    m_color_map["Mspecular"] = INTERNAL;
    m_color_map["Mdistance"] = INTERNAL;
    m_color_map["Mshine"] = INTERNAL;
    m_color_map["ColorRGB"] = INTERNAL;
    m_color_map["Camera"] = GLOBAL;
    m_color_map["Ray"] = GLOBAL;
    m_color_map["SphereSDF"] = INTERNAL;
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


