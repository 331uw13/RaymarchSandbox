#ifndef AMBIENT3D_LIGHT_HPP
#define AMBIENT3D_LIGHT_HPP

#include <cstddef>

#include "raylib.h"
#include "raymath.h"


namespace AM {

    struct Light {
        
        Vector3  pos;
        Color    color;
        float    cutoff;
        float    strength;

        size_t   id { 0 };
        bool     force_update { false };

        bool equal(const Light& rhs) {
            return (
                    FloatEquals(this->pos.x, rhs.pos.x) &&
                    FloatEquals(this->pos.y, rhs.pos.y) &&
                    FloatEquals(this->pos.z, rhs.pos.z) &&
                    (this->color.r == rhs.color.r) &&
                    (this->color.g == rhs.color.g) && 
                    (this->color.b == rhs.color.b) &&
                    FloatEquals(this->strength, rhs.strength) &&
                    FloatEquals(this->cutoff, rhs.cutoff)
                    ); 
        }

    };
}


#endif
