#ifndef AMBIENT3D_ITEM_HPP
#define AMBIENT3D_ITEM_HPP


#include "item_base.hpp"
#include "renderable.hpp"
#include "raylib.h"


namespace AM {

    class Item : public AM::ItemBase {
        public:

            Renderable* renderable { NULL };
            Texture inv_texture; // Inventory texture.  TODO: Create tool for this

            float inactive_time;

        private:

    };

};



#endif
