#ifndef AMBIENT3D_NETWORK_PLAYER_HPP
#define AMBIENT3D_NETWORK_PLAYER_HPP


#include <raylib.h>

namespace AM {

    // Different class for other players in the server.
    // The 'Player' class has alot of functionality the others do not need.

    class N_Player {
        public:

            // ------- Server side -------

            int        id { 0 };
            Vector3    pos { 0.0f, 0.0f, 0.0f };
            float      cam_yaw { 0.0f };
            float      cam_pitch { 0.0f };
            int        anim_id { 0 };

            
        private:

    };

}






#endif
