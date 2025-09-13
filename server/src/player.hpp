#ifndef AMBIENT3D_SERVER_PLAYER_HPP
#define AMBIENT3D_SERVER_PLAYER_HPP


#include "tcp_session.hpp"
#include "vec3.hpp"


namespace AM {
    class Server;

    class Player {
        public:

            Player(std::shared_ptr<TCP_session> _tcp_session);
           
            std::shared_ptr<TCP_session> tcp_session;

            int id           { -1 };
            Vec3 pos         { 0, 0, 0 };
            float cam_yaw    { 0 };
            float cam_pitch  { 0 };
            int anim_id      { 0 };


        private:

    };


};


#endif
