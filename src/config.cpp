#include <raylib.h>

#include "rmsb.hpp"
#include "config.hpp"
#include "logfile.hpp"
#include "libs/glad.h"

void Config::read_values_before_init(const INIReader& ini, Settings* settings) {

    settings->imgui_font = ini.GetString(
            "font_settings",
            "imgui_font",
            "./fonts/AdwaitaSans-Regular.ttf"
            );


    settings->editor_font = ini.GetString(
            "font_settings",
            "editor_font",
            "./fonts/Px437_IBM_Model3x_Alt4.ttf"
            );
}


void Config::read_values_after_init(const INIReader& ini, RMSB* rmsb) {

    // Note:
    // The errors happening with config file reading
    // should be displayed to the user with rmsb->loginfo 
    // and written to logfile.

    rmsb->fps_limit = ini.GetInteger(
            "render_settings",
            "fps_limit", 125);

    rmsb->fov = ini.GetReal(
            "render_settings",
            "fov", 60.0);  

    rmsb->hit_distance = ini.GetReal(
            "render_settings",
            "hit_distance", 0.001);
    
    rmsb->max_ray_len = ini.GetReal(
            "render_settings",
            "max_ray_length", 500.0);
    
    rmsb->ao_step_size = ini.GetReal(
            "render_settings",
        "ao_step", 0.01);
    
    rmsb->ao_num_samples = ini.GetInteger(
            "render_settings",
            "ao_samples", 32);
    
    rmsb->ao_falloff = ini.GetReal(
            "render_settings",
            "ao_falloff", 3.0);
    
    rmsb->translucent_step_size = ini.GetReal(
            "render_settings",
            "translucent_step", 0.1);


    std::string res_str = ini.GetString(
            "render_settings",
            "render_resolution", "");
    if(res_str.empty()) {
        rmsb->loginfo(RED, "Could not find render resolution setting! Set to 'HALF'");
        append_logfile(ERROR, "Could not find render resolution setting.");
        res_str = "HALF";
    }

    int res_x = rmsb->monitor_width;
    int res_y = rmsb->monitor_height;

    if(res_str == "HALF") {
        res_x /= 2;
        res_y /= 2;
    }
    else
    if(res_str == "LOW") {
        res_x /= 3;
        res_y /= 3;
    }
    if(res_str == "CUSTOM") {
        res_x = ini.GetInteger(
                "render_settings",
                "custom_render_resolution_X", 0);
        
        res_y = ini.GetInteger(
                "render_settings",
                "custom_render_resolution_Y", 0);

        if(res_x <= 0) {
            rmsb->loginfo(RED, "Custom resolution X is invalid, set to half.");
            append_logfile(ERROR, "Custom resolution X is invalid. Too small.");
            res_x = rmsb->monitor_width/2;
        }
        if(res_y <= 0) {
            rmsb->loginfo(RED, "Custom resolution Y is invalid, set to half.");
            append_logfile(ERROR, "Custom resolution Y is invalid. Too small.");
            res_y = rmsb->monitor_height/2;
        }

        if(res_x > rmsb->monitor_width) {
            res_x = rmsb->monitor_width;
        }
        if(res_y > rmsb->monitor_height) {
            res_y = rmsb->monitor_height;
        }
    }



    rmsb->render_texture = rmsb->create_empty_texture(
            res_x, res_y, GL_RGBA16F);
    
    SetTargetFPS(rmsb->fps_limit);
}



