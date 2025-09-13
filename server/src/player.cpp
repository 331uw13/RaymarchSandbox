#include "player.hpp"





AM::Player::Player(std::shared_ptr<TCP_session> _tcp_session)
: tcp_session(std::move(_tcp_session)) 
{

}







