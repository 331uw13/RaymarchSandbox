#ifndef AMBIENT3D_GUI_MODULE_HPP
#define AMBIENT3D_GUI_MODULE_HPP


// GuiModule will let the 'AM::State' 
// handle focus and user input events for
// modules automatically. Each module can gain focus with a function call.
// New gui modules must be registered with AM::State::register_gui_module()
// The focus can be set for module with AM::State::module_gain_focus()
// see ambient.hpp for more info.


#include <raylib.h>

namespace AM {

    enum GuiModuleID : int {
        NO_MODULE_ID = 0,
        CHATBOX
        // ...

    };

    class GuiModule {
        public:

            enum RenderOPT {
                ALWAYS,
                WHEN_FOCUSED
            };

            GuiModule(GuiModuleID id, RenderOPT opt) {
                m_id = id;
                m_render_option = opt;
            }

            // These functions are implemented 
            // by the module which extends GuiModule.
            virtual void module__char_input(int/* key*/){}
            virtual void module__render(Font*) {}

            GuiModuleID get_id()             { return m_id; }
            RenderOPT   get_render_option()  { return m_render_option; }
            
            bool   has_focus { false };

        private:

            GuiModuleID m_id { NO_MODULE_ID };
            RenderOPT m_render_option;

    };

};






#endif
