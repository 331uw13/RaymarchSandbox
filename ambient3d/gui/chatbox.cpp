

#include "chatbox.hpp"

namespace {
    static constexpr Color CHAT_BG_COLOR
        = Color(30, 30, 30, 80);
    
    static constexpr Color CHAT_FOCUS_COLOR
        = Color(80, 200, 80, 200);
    
    static constexpr Color CHAT_SEPARATOR_LINE_FOCUSED_COLOR
        = Color(80, 80, 80, 200);
    
    static constexpr Color CHAT_SEPARATOR_LINE_COLOR
        = Color(50, 50, 50, 200);

    static constexpr Color MSG_BG_COLOR_LIGHT
        = Color(50, 50, 50, 100);
    
    static constexpr Color MSG_BG_COLOR_DARK
        = Color(40, 40, 40, 100);

};

void AM::Chatbox::module__render(Font* font) {  

    int font_size = 20;
    //int win_width = GetScreenWidth();
    int win_height = GetScreenHeight();

    int this_x = 10;
    int this_y = (win_height - this->height) - 30;

    DrawRectangle(this_x, this_y, this->width, this->height+font_size, ::CHAT_BG_COLOR);
    if(this->has_focus) {
        DrawRectangleLines(this_x-2, this_y-2, this->width+4, this->height+4+font_size, ::CHAT_FOCUS_COLOR);
    }

    constexpr int paddnX = 10;
    constexpr int paddnY = 5;

    // The text will be offseted so old messages go up.
    int text_y_offset = paddnY;
    int text_x_offset = paddnX;

    for(size_t i = 0; i < m_msg_buffer.size(); i++) {
        const Vector2 text_pos = Vector2(
                    this_x + text_x_offset, 
                    this_y + text_y_offset);

        // Message background.
        DrawRectangleRounded(
                Rectangle(
                    text_pos.x - paddnX + 5,
                    text_pos.y,
                    this->width - paddnX, font_size
                    ),
                0.5f,
                8,
                (i%2 == 0) ? ::MSG_BG_COLOR_LIGHT : ::MSG_BG_COLOR_DARK
                );

        DrawTextEx(*font, 
                m_msg_buffer[i].data.c_str(), 
                text_pos,
                (float)font_size,
                0.8f,
                m_msg_buffer[i].color
                );

        text_y_offset += font_size+2;
    }


    DrawLine(paddnX, 
             this_y + this->height,
             paddnX + this->width,
             this_y + this->height,
             this->has_focus 
                ? ::CHAT_SEPARATOR_LINE_FOCUSED_COLOR 
                : ::CHAT_SEPARATOR_LINE_COLOR);

    // Draw text inputs.
    DrawTextEx(*font, 
            this->text_input.c_str(), 
            Vector2(paddnX+5, this_y + this->height),
            (float)font_size,
            0.8f,
            WHITE
            );



}
 
void AM::Chatbox::module__char_input(int key) {
    if(key >= 0x20 && key <= 0x7E) { 
        this->text_input.push_back(key);
    }
}

void AM::Chatbox::push_message(uint8_t red, uint8_t grn, uint8_t blu, const std::string& msg) {
    if(msg.empty()) { return; }

    m_msg_buffer.push_back(ChatMsg{ Color(red, grn, blu, 255), msg });
    // TODO: Make sure the chat buffer memory doesnt grow too big.
}



