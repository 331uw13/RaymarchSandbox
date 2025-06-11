#ifndef RAYMARCH_SANDBOX_GUI_HPP
#define RAYMARCH_SANDBOX_GUI_HPP

#define GUI_WIDTH 355
#define GUI_HEIGHT 600
#define FUNCTIONS_VIEW_WIDTH 750



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
    private:

};


#endif
