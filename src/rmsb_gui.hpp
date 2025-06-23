#ifndef RAYMARCH_SANDBOX_GUI_HPP
#define RAYMARCH_SANDBOX_GUI_HPP

#define GUI_WIDTH 420
#define GUI_HEIGHT 600
#define FUNCTIONS_VIEW_WIDTH 750

#include <initializer_list>
#include <string_view>
#include <vector>
#include <string>

class RMSB;
struct uniform_t;


class RMSBGui {
    public:

        void init();
        void quit();
        void update();
        void render(RMSB* rmsb);

        bool view_functions;
        bool open;

        
        // Returns the option index that was chosen by user.
        int ask_question(const char* question, std::initializer_list<std::string_view> options);

    private:

};




#endif
