#ifndef AMBIENT3D_UDP_SESSION_HPP
#define AMBIENT3D_UDP_SESSION_HPP


#include <asio.hpp>
using namespace asio::ip;


#include "packet_writer.hpp"


namespace AM {
    class Server;

    class UDP_handler {
        public:

            UDP_handler(asio::io_context& io_context, uint16_t port)
                : m_socket(io_context, udp::endpoint(udp::v4(), port)) {}

            void start(AM::Server* server) {
                m_server = server;
                m_do_read();
            }


            Packet packet;
            void send_packet(int player_id);


        private:
            Server* m_server;

            void m_handle_received_packet(size_t sizeb);
            void m_do_read();
            void m_do_write();


            udp::endpoint m_sender_endpoint; // <- "temporary" see m_do_read().
            std::map<int/*player_id*/, udp::endpoint> m_recv_endpoints;

            udp::socket m_socket;

            char m_data[AM::MAX_PACKET_SIZE];

    };

};



#endif
