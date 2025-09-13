#ifndef AMBIENT3D_ANIMATION_HPP
#define AMBIENT3D_ANIMATION_HPP

#include <cstdint>
#include "raylib.h"


namespace AM {

    class Animation {
        public:

            void load(const char* path);
            void unload();

            void set_animation_speed  (uint32_t anim_index, float anim_speed);
            void update               (uint32_t anim_index, float frame_time, Model* model);

            uint32_t  num_animations()  { return m_num_anims; }
            bool      is_loaded()       { return m_loaded; }

        private:
            
            ModelAnimation*  m_anim_data { NULL };
            
            bool      m_loaded { false };
            uint32_t  m_num_anims { 0 }; // Number of animations.
            uint32_t  m_prev_anim_index { 0 };
            int       m_current_frame { 0 };
            float     m_animation_timer;
            float*    m_animation_speeds;
    };

};



#endif
