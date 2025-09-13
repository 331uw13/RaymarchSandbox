#ifndef AMBIENT3D_CHATBOX_HPP
#define AMBIENT3D_CHATBOX_HPP


#include <vector>
#include <string>
#include <cstdint>
#include <raylib.h>

#include "gui_module.hpp"


namespace AM {

    struct ChatMsg {
        Color        color;
        std::string  data;
    };

    
    class Chatbox : public GuiModule {
        public: 
            using GuiModule::GuiModule;


            uint32_t width  { 500 };
            uint32_t height { 200 };

            //void render(Font* font, int font_size);

            void push_message(uint8_t red, uint8_t grn, uint8_t blu, const std::string& msg);


            void module__char_input(int key) override;
            void module__render(Font* font) override;

            std::string text_input;

        private:

            std::vector<ChatMsg> m_msg_buffer;

    };
};





#endif
