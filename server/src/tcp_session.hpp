#ifndef AMBIENT3D_TCP_SESSION_HPP
#define AMBIENT3D_TCP_SESSION_HPP


#include <memory>
#include <vector>
#include <string>

#include <asio.hpp>
using namespace asio::ip;


// From '../../shared'
#include "packet_writer.hpp"


namespace AM {
    class Server;

    class TCP_session : public std::enable_shared_from_this<TCP_session> {
        public:
            TCP_session(tcp::socket socket, AM::Server* server, int player_id);

            Packet packet;
            


            void start() { m_do_read(); }
            void send_packet();
            
            int  player_id;


        private:

            std::vector<std::string> m_write_buffer;

            void m_do_read();

            tcp::socket  m_socket;
            Server*      m_server;

            void m_handle_received_packet(size_t sizeb);
            char m_data[AM::MAX_PACKET_SIZE]; // <- Received data. (TODO: Should probably rename.)
            
    };
};




#endif
