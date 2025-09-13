#ifndef AMBIENT3D_PLAYER_HPP
#define AMBIENT3D_PLAYER_HPP

#include <cstdint>

#include "raylib.h"
#include "rcamera.h"

#include "anim_ids.hpp"


namespace AM {
    class State;
    class Player {
        public:
            Player();
            ~Player();
            
            Camera   cam;
            Vector3  pos { 0, 0, 0 };
            Vector3  vel { 0, 0, 0 }; // Velocity.
            Vector3  pos_prevframe { 0, 0, 0 };
            bool     noclip { false };
            float    noclip_speed { 0 };
            bool     on_ground { false };
            float    gravity { 0 };
            float    speed { 0 };
            float    height { 0 };
            float    cam_sensetivity { 0.0f };
            float    cam_yaw { 0.0f };
            float    cam_pitch { 0.0f };
            Vector3  movement { 0, 0, 0 };
            Vector3  forward { 0, 0, 0 };
            int      chunk_x { 0 };
            int      chunk_z { 0 };
            int8_t   num_jumps_in_air { 0 };
            Vector3  terrain_normal { 0, 0, 0 };
            bool     is_moving { false };
            bool     is_running { false };
            int      anim_id { 0 };

            void update_movement(State* st, bool handle_user_input = true);
            void update_camera();
            void update_animation();

            void jump();

        private:

            void m_update_gravity(State* st);
            void m_update_slide();

            float m_slide_boost { false };
            bool m_jumped { false };
            int  m_num_jumps_left { 0 };
            bool m_sliding { false };

            // Used for sliding.
            // It is the Y axis velocity while in air
            // and may be reset by 'm_update_slide'
            float m_Yvelocity_in_air { 0.0f };
    };
};




#endif
