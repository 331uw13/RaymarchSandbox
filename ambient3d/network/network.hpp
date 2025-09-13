#ifndef AMBIENT3D_NETWORKING_HPP
#define AMBIENT3D_NETWORKING_HPP

#include <cstdint>
#include <thread>
#include <functional>
#include <map>

#include <deque>
#include <asio.hpp>

#include "packet_writer.hpp"
#include "network_player.hpp"
#include "../item_manager.hpp"

#include "../gui/chatbox.hpp"



namespace AM {

    struct NetConnectCFG {
        const char* host;
        const char* tcp_port;
        const char* udp_port;

        // Called when data is received.
        std::function<void(
                uint8_t, // Red
                uint8_t, // Green
                uint8_t, // Blue
                const std::string&)> msg_recv_callback { NULL };
    };

    enum NetProto {
        TCP,
        UDP
    };



    class Network {

        public:

            Network(asio::io_context& io_context, const NetConnectCFG& cfg);
            ~Network() {}

            void assign_item_manager(AM::ItemManager* item_manager) {
                m_engine_item_manager = item_manager;
            }

            // The packet can be written with functions from
            // './shared/packet_writer.*'
            // This is going to be saved here so it dont need to be
            // always allocated again on the stack.
            Packet packet;
            
            void   send_packet(AM::NetProto proto);

            std::map<int/* player_id*/, N_Player>  players;

            bool is_connected() { return m_connected; }
            void close(asio::io_context& io_context);

            int player_id;
            //UUID uuid;

        private:
            bool m_connected { false };

            std::function<void(
                    uint8_t, // Red
                    uint8_t, // Green
                    uint8_t, // Blue
                    const std::string&)> m_msg_recv_callback;

            ItemManager* m_engine_item_manager;

            bool m_udp_data_ready_to_send;
            bool m_tcp_data_ready_to_send;
           
            std::thread m_event_handler_th;

            asio::ip::tcp::socket m_tcp_socket;
            void m_do_write_tcp();
            void m_do_read_tcp();


            asio::ip::udp::socket m_udp_socket;
            asio::ip::udp::endpoint m_udp_sender_endpoint;
            void m_do_read_udp();
            void m_do_write_udp();

            char m_udprecv_data[AM::MAX_PACKET_SIZE];
            char m_tcprecv_data[AM::MAX_PACKET_SIZE];
            
            void m_handle_tcp_packet(size_t sizeb);
            void m_handle_udp_packet(size_t sizeb);
    };
};


#endif
