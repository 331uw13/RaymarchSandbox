#include <cstdio>


#include "player.hpp"
#include "chunk.hpp"
#include "util.hpp"
#include "ambient3d.hpp"

#include "raymath.h"



AM::Player::Player() {
    this->cam.position = Vector3(0.0f, 0.0f, 0.0f);
    this->cam.target = Vector3(0.1f, 0.0f, 0.0f);
    this->cam.up = Vector3(0.0f, 1.0f, 0.0f);
    this->cam.fovy = 70.0f;
    this->cam.projection = CAMERA_PERSPECTIVE;
    this->vel = Vector3(0.0f, 0.0f, 0.0f);
    this->pos = Vector3(0.0f, 10.0f, 1.0f);
    this->speed = 40.0f;
    this->cam_sensetivity = 0.0028f;
    this->noclip = false;
    this->noclip_speed = 250.0f;
    this->height = 1.85f;
    this->gravity = 70.0f;
    this->on_ground = false;
    this->num_jumps_in_air = 2;
    this->pos_prevframe = this->pos;
}

AM::Player::~Player() {
}
 
        
void AM::Player::update_animation() {

    // IDLE is default.
    this->anim_id = AM::AnimID::IDLE;

    if(this->is_moving) {
        this->anim_id = AM::AnimID::WALKING;
    }
    if(this->is_running) {
        this->anim_id = AM::AnimID::RUNNING;
    }
}


void AM::Player::update_movement(State* st, bool handle_user_input) {
    const float dt = GetFrameTime();
    Vector3 cam_dir = Vector3(
                cos(this->cam_pitch) * sin(this->cam_yaw),
                sin(this->cam_pitch),
                cos(this->cam_pitch) * cos(this->cam_yaw)
                );

    // TODO: Do not hardcode values.

    constexpr Vector3 UP = { 0.0f, 1.0f, 0.0f };

    this->is_moving = false;
    this->is_running = false;

    cam_dir = Vector3Normalize(cam_dir);
    Vector3 right = Vector3Normalize(Vector3CrossProduct(cam_dir, UP));
    this->forward = Vector3Normalize(Vector3CrossProduct(right, UP));

    float speed = (this->noclip) ? this->noclip_speed : this->speed;

    if(!this->noclip && IsKeyDown(KEY_LEFT_SHIFT)) {
        speed *= 3.0f;
        this->is_running = true;
    }
    else
    if(this->noclip && IsKeyDown(KEY_LEFT_ALT)) {
        speed *= 10.0; // Noclip speed multiplier.
    }
    

    speed *= dt;

    if(handle_user_input) {
        if(IsKeyDown(KEY_W)) {
            this->movement.x -= this->forward.x * speed;
            this->movement.z -= this->forward.z * speed;
            if(this->noclip) {
                this->movement.y += cam_dir.y * speed;
            }
        }
        else
        if(IsKeyDown(KEY_S)) {
            this->movement.x += this->forward.x * speed;
            this->movement.z += this->forward.z * speed;
            if(this->noclip) {
                this->movement.y -= cam_dir.y * speed;
            }
        }
        if(IsKeyDown(KEY_D)) {
            this->movement.x += right.x * speed;
            this->movement.z += right.z * speed;
        }
        else
        if(IsKeyDown(KEY_A)) {
            this->movement.x -= right.x * speed;
            this->movement.z -= right.z * speed;
        }
    }

    if(!this->noclip) {
        if(handle_user_input && IsKeyPressed(KEY_SPACE)) {
            this->jump();
        }
        m_update_gravity(st);
        m_update_slide();
    }
    else 
    if(handle_user_input) {
        if(IsKeyDown(KEY_SPACE)) {
            this->movement.y += speed;
        }
        else
        if(IsKeyDown(KEY_LEFT_SHIFT)) {
            this->movement.y -= speed;
        }
        this->vel.y = 0;
    }

    this->pos += this->movement * dt;

    float F = pow(0.95, dt * 120);
    this->movement.x *= F;
    this->movement.y *= F;
    this->movement.z *= F;

    this->cam.position = this->pos;
    this->cam.target = Vector3Add(this->pos, cam_dir);

    this->chunk_x = (int)floor(this->pos.x / (AM::CHUNK_SIZE * AM::CHUNK_SCALE));
    this->chunk_z = (int)floor(this->pos.z / (AM::CHUNK_SIZE * AM::CHUNK_SCALE));

    constexpr float MOVING_TRESHOLD = 0.3f;
    if(Vector3Length(this->movement) > MOVING_TRESHOLD) {
        this->is_moving = true;
    }

}

void AM::Player::m_update_slide() {
    const float dt = GetFrameTime();
    if(m_sliding || (!m_sliding && !this->on_ground)) {
        this->movement.x -= this->forward.x * (dt * m_slide_boost);
        this->movement.z -= this->forward.z * (dt * m_slide_boost);
    }

    constexpr float slide_boost_mult = 200.0f;
    constexpr float y_velocity_mult = 10.0f;
    constexpr float active_treshold = 0.95f;
    const float boost_dampen = pow(0.98f, 120.0f*dt);
    
    if(!IsKeyDown(KEY_LEFT_CONTROL)) {
        m_slide_boost *= boost_dampen;
        m_sliding = false;
        return;
    }
    if(!this->on_ground) {
        m_slide_boost *= boost_dampen;
        m_sliding = false;
        return;
    }

    float diff = Vector3DotProduct(this->terrain_normal, Vector3(0.0f, 1.0f, 0.0f));  
    bool going_down = (this->pos_prevframe.y > this->pos.y); 
    if(going_down && (diff < active_treshold)) {
        m_slide_boost += dt * slide_boost_mult;
        m_slide_boost += m_Yvelocity_in_air * y_velocity_mult;
        m_Yvelocity_in_air = 0;
    }
    else {
        m_slide_boost *= boost_dampen;
    }
 
    AMutil::clamp<float>(m_slide_boost, 0.0f, 1000.0f);


    m_sliding = true;
}

void AM::Player::jump() {
    if(m_num_jumps_left <= 0) {
        return;
    }

    m_sliding = false;

    this->vel.y = -20;
    m_num_jumps_left--;
    m_jumped = true;
}

void AM::Player::m_update_gravity(State* st) {
    this->pos_prevframe = this->pos;
   
    float terrain_level = st->terrain.get_height(this->pos.x, this->pos.z, &this->terrain_normal);
    this->on_ground = ((this->pos.y - this->height) < terrain_level);

    if(m_sliding) {
        this->pos.y = terrain_level + (this->height - 0.01f);
        this->on_ground = true;
        return;
    }

    if(this->on_ground && !m_jumped) {
        this->pos.y = terrain_level + (this->height - 0.01f);
        this->vel.y = 0;
        m_num_jumps_left = this->num_jumps_in_air;
    }
    else {
        const float dt = GetFrameTime();
        this->vel.y += dt * (this->gravity);
        this->pos.y -= this->vel.y * dt;
        m_Yvelocity_in_air = this->vel.y;
        AMutil::clamp<float>(m_Yvelocity_in_air, 0.0f, 30.0f);
    }
        
    m_jumped = false;
}
 
void AM::Player::update_camera() {
    Vector2 md = GetMouseDelta();
    this->cam_yaw += (-md.x * this->cam_sensetivity);
    this->cam_pitch += (-md.y * this->cam_sensetivity);
    
    AMutil::clamp<float>(this->cam_pitch, -1.5f, 1.5f);
}
            

