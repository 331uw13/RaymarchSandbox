#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "libs/INIReader.h"



namespace Config 
{

    // Settings read from '..before_init'
    struct Settings {
        std::string editor_font;
        std::string imgui_font;
        float editor_font_size;
        float imgui_font_size;
    };


    void read_values_before_init(const INIReader& ini, Settings* settings);
    void read_values_after_init(const INIReader& ini, RMSB* rmsb);

};



#endif
