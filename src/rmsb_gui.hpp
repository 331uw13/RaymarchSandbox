#ifndef RAYMARCH_SANDBOX_GUI_HPP
#define RAYMARCH_SANDBOX_GUI_HPP

#define GUI_WIDTH 400
#define GUI_HEIGHT 500
#define FUNCTIONS_VIEW_WIDTH 470


class RMSB;

class RMSBGui {
    public:

        void init();
        void quit();
        void update();
        void render(RMSB* rmsb);

        bool view_ilibsrc; // Show InternalLib Source.
        bool view_functions;
        bool open;
    private:

};


#endif
