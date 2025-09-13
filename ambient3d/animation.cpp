#include <cstdio>
#include <cstring>
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "animation.hpp"



void AM::Animation::load(const char* path) {
    if(m_loaded) {
        fprintf(stderr, "WARNING! %s: Animation for \"%s\" seems to be already loaded.\n",
                __func__, path);
        return;
    }

    int anim_count = 0;
    m_anim_data = NULL;
    m_anim_data = LoadModelAnimations(path, &anim_count);

    m_loaded = ((anim_count > 0) && m_anim_data);
    m_num_anims = (uint32_t)anim_count;

    if(!m_loaded) {
        return;
    }
    
    m_animation_speeds = new float[m_num_anims];
    for(uint32_t i = 0; i < m_num_anims; i++) {
        m_animation_speeds[i] = 0.05f; // Default value. It is probably changed by user.
    }
}

void AM::Animation::unload() {
    if(!m_loaded) { 
        fprintf(stderr, "WARNING! %s: Trying to unload already unloaded animation.\n",
                __func__);
        return;
    }

    if(m_animation_speeds) {
        delete[] m_animation_speeds;
        m_animation_speeds = NULL;
    }
    UnloadModelAnimations(m_anim_data, m_num_anims);
    m_anim_data = NULL;
    m_loaded = false;
}

void AM::Animation::set_animation_speed(uint32_t anim_index, float anim_speed) {
    if(!m_loaded) {
        fprintf(stderr, "ERROR! %s: Animation must be loaded first.\n",
                __func__);
        return;
    }
    if(anim_index >= m_num_anims) {
        fprintf(stderr, "ERROR! %s: anim_index(%i) is out of bounds.\n",
                __func__, anim_index);
        return;
    }
    m_animation_speeds[anim_index] = anim_speed;
}

void AM::Animation::update(uint32_t anim_index, float frame_time, Model* model) {
    if(!m_loaded) { return; }
    if(anim_index >= m_num_anims) { return; }
    
    if(model->boneCount != m_anim_data[anim_index].boneCount) {
        fprintf(stderr, "ERROR! Number of bones in model and animation doesnt match.\n");
        return;
    }

    const bool anim_index_changed = (m_prev_anim_index != anim_index);
    
    m_animation_timer += frame_time;
    if((m_animation_timer < m_animation_speeds[anim_index]) && !anim_index_changed) {
        m_prev_anim_index = anim_index;
        return;
    }
        
    m_animation_timer = 0.0f;

    if(!anim_index_changed) {
        m_current_frame++;
    }
    else {
        m_current_frame = 0;
    }

    m_current_frame %= m_anim_data[anim_index].frameCount;
    UpdateModelAnimation(*model, m_anim_data[anim_index], m_current_frame);

    
    m_prev_anim_index = anim_index;
}



