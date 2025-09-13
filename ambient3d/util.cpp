#include <cstdio>

#include "raylib.h"
#include "rlgl.h"

#include "util.hpp"
#include "ambient3d.hpp"


static int AMutil__randomgen (int64_t* seed) {
    *seed = 0x343FD * (*seed) + 0x269EC3;
    return (*seed >> 16) & 0x7FFF;
}


float AMutil::normalize(float t, float min, float max) {
    return (t - min) / (max - min);
}

float AMutil::lerp(float t, float min, float max) {
    return (max - min) * t + min;
}

float AMutil::map(float t, float src_min, float src_max, float dst_min, float dst_max) {
    return (t - src_min) * (dst_max - dst_min) / (src_max - src_min) + dst_min;
}

int AMutil::randomi(int min, int max, int64_t* seed) {
    return AMutil__randomgen(seed) % (max - min) + min;
}

float AMutil::randomf(float min, float max, int64_t* seed) {
    return ((float)AMutil__randomgen(seed) / ((float)0x7FFF / (max - min))) + min;
}

std::string AMutil::combine_constchar(std::initializer_list<const char*> list) {
    std::string res = "";
    for(const char* c : list) {
        res += c;
    }
    return res;
}

std::string AMutil::combine_files(std::initializer_list<const char*> filenames) {
    std::string res = "";
    for(const char* fname : filenames) {
        if(!FileExists(fname)) {
            fprintf(stderr, "ERROR! %s: \"%s\" doesnt exist, or no permission to read.\n", 
                    __func__, fname);
            continue;
        }

        char* content = LoadFileText(fname);
        res += content;
        UnloadFileText(content);
    }
    return res;
}


void AMutil::resample_texture(
        AM::State* st,
        RenderTexture2D to,
        RenderTexture2D from,
        int shader_idx,
        int src_width, int src_height,
        int dst_width, int dst_height
){
    BeginTextureMode(to);
    ClearBackground(Color(0, 0, 0, 0));

    if(shader_idx >= 0) {
        BeginShaderMode(st->shaders[shader_idx]);
    }

    if(src_width < 0)  { src_width = from.texture.width;    }
    if(src_height < 0) { src_height = from.texture.height;  }
    if(dst_width < 0)  { dst_width = to.texture.width;      }
    if(dst_height < 0) { dst_height = to.texture.height;     }

    DrawTexturePro(
            from.texture,
            (Rectangle){ 0, 0, (float)src_width, (float)-src_height },
            (Rectangle){ 0, 0, (float)dst_width, (float)-dst_height },
            (Vector2){ 0, 0 }, 0, WHITE);

    if(shader_idx >= 0) {
        EndShaderMode();
    }
    EndTextureMode();
}

