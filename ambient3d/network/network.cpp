#include <cstdio>
#include <string>
#include <iostream>

#include "network.hpp"
#include "packet_parser.hpp"



void AM::Network::m_handle_tcp_packet(size_t sizeb) {
    AM::PacketID packet_id = AM::parse_network_packet(m_tcprecv_data, sizeb);
    if(packet_id == AM::PacketID::NONE) {
        return;
    }

    switch(packet_id) {
        case AM::PacketID::CHAT_MESSAGE:
            printf("[CHAT]: %s\n", m_tcprecv_data);
            m_msg_recv_callback(255, 255, 255, m_tcprecv_data);
            break;

        case AM::PacketID::SERVER_MESSAGE:
            printf("[SERVER]: %s\n", m_tcprecv_data);
            m_msg_recv_callback(255, 200, 50, m_tcprecv_data);
            break;

        case AM::PacketID::SAVE_ITEM_LIST:
            //m_engine_item_manager->assign_item_list(json::parse(m_tcprecv_data));
            printf("[NETWORK]: Got item list from server.\n");
            break;

        case AM::PacketID::PLAYER_ID:
            if(sizeb != sizeof(this->player_id)) {
                fprintf(stderr, "%s: ERROR! Packet size(%li) doesnt match expected size "
                        "for PLAYER_ID\n", __func__, sizeb);
                return;
            }
            memmove(&this->player_id, m_tcprecv_data, sizeof(this->player_id));

            // Now send the received player id via UDP
            // so the server can save the endpoint.
            AM::packet_prepare(&this->packet, AM::PacketID::PLAYER_ID);
            AM::packet_write_int(&this->packet, { this->player_id });
            this->send_packet(AM::NetProto::UDP);
            break;

        case AM::PacketID::PLAYER_ID_HAS_BEEN_SAVED:
            m_msg_recv_callback(120, 255, 120, "Connected!");
            break;

    }
}


void AM::Network::m_handle_udp_packet(size_t sizeb) {
    AM::PacketID packet_id = AM::parse_network_packet(m_udprecv_data, sizeb);

    switch(packet_id) {

        case AM::PacketID::ITEM_UPDATE:
            if(sizeb < (sizeof(int)*2 + sizeof(float)*3)) {
                fprintf(stderr, "%s: ERROR! Packet size(%li) doesnt match expected size "
                        "for ITEM_UPDATE\n", __func__, sizeb);
                return;
            }
            {
                static AM::ItemBase itembase;

                size_t byte_offset = 0;
                while(byte_offset < sizeb) {

                    memmove(&itembase.uuid, &m_udprecv_data[byte_offset], sizeof(int));
                    byte_offset += sizeof(int);
                    
                    memmove(&itembase.id, &m_udprecv_data[byte_offset], sizeof(int));
                    byte_offset += sizeof(int);
                    
                    memmove(&itembase.pos_x, &m_udprecv_data[byte_offset], sizeof(float)*3);
                    byte_offset += sizeof(float)*3;

                    // Read item entry_name
                    memset(itembase.entry_name, 0, AM::ITEM_MAX_ENTRYNAME_SIZE);
                    size_t ename_idx = 0;
                    while(true) {
                        char byte = m_udprecv_data[byte_offset];
                        if(byte == AM::PACKET_DATA_SEPARATOR) {
                            byte_offset++; // Increment byte_offset here too 
                                           // or else next item will have
                                           // the separator byte in the name
                            break;
                        }

                        itembase.entry_name[ename_idx] = byte;

                        if(ename_idx++ >= AM::ITEM_MAX_ENTRYNAME_SIZE) {
                            fprintf(stderr, "ERROR! %s: Unexpectedly long entry name (%s)\n",
                                    __func__, itembase.entry_name);
                            return;
                        }
                        if(byte_offset++ >= sizeb) {
                            break;
                        }
                    }

                    // Add the item to queue to be loaded or only updated.
                    m_engine_item_manager->add_itembase_to_queue(itembase);
                }
            }
            break;

        case AM::PacketID::PLAYER_MOVEMENT_AND_CAMERA:
            if(sizeb != AM::PacketSize::PLAYER_MOVEMENT_AND_CAMERA) {
                fprintf(stderr, "%s: ERROR! Packet size(%li) doesnt match expected size "
                        "for PLAYER_MOVEMENT_AND_CAMERA\n", __func__, sizeb);
                return;
            }
            {
                int player_id = -1;
                memmove(&player_id, m_udprecv_data, sizeof(player_id));

                N_Player player;
                player.id = player_id;

                // Copy the player data if it already exists.
                const auto player_search = this->players.find(player_id);
                if(player_search != this->players.end()) {
                    player = player_search->second;
                }

                size_t offset = sizeof(player_id);
                
                memmove(&player.pos, m_udprecv_data+offset, sizeof(float)*3);
                offset += sizeof(float)*3;

                memmove(&player.cam_yaw, m_udprecv_data+offset, sizeof(float));
                offset += sizeof(float);
                
                memmove(&player.cam_pitch, m_udprecv_data+offset, sizeof(float));
                offset += sizeof(float);

                memmove(&player.anim_id, m_udprecv_data+offset, sizeof(int));
                //offset += sizeof(int);

                // Insert player data if not in hashmap
                // or replace existing one.
                this->players[player_id] = player;
            }
            break;

    }

}





AM::Network::Network(asio::io_context& io_context, const NetConnectCFG& cfg)
    : m_tcp_socket(io_context),
      m_udp_socket(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0))
{
    if(!cfg.msg_recv_callback) {
        fprintf(stderr, "WARNING! %s: No message received callback.\n",__func__);
    }

    try {
        m_msg_recv_callback = cfg.msg_recv_callback;

        asio::ip::tcp::resolver tcp_resolver(io_context);
        asio::connect(m_tcp_socket, tcp_resolver.resolve(cfg.host, cfg.tcp_port));

        asio::ip::udp::resolver udp_resolver(io_context);
        m_udp_sender_endpoint = *udp_resolver.resolve(cfg.host, cfg.udp_port).begin();
    
        memset(m_tcprecv_data, 0, AM::MAX_PACKET_SIZE);
        memset(m_udprecv_data, 0, AM::MAX_PACKET_SIZE);

        m_connected = true;
        m_do_read_tcp();
        m_do_read_udp();

        // Start event handler.
        m_event_handler_th = std::thread([](asio::io_context& context){
                context.run();
                }, std::ref(io_context));

    }
    catch(const std::exception& e) {
        fprintf(stderr, "%s: %s\n",
                __func__, e.what());
    }
}

void AM::Network::close(asio::io_context& io_context) {
    printf("Closing network connection...\n");
    io_context.stop();
    m_event_handler_th.join();
    m_connected = false;
}


void AM::Network::send_packet(AM::NetProto proto) {

    // TODO: Check packet status before sending it.
    
    if(proto == AM::NetProto::TCP) {
        m_tcp_data_ready_to_send = true;
        m_do_write_tcp();
    }
    else 
    if(proto == AM::NetProto::UDP) {
        m_udp_data_ready_to_send = true;
        m_do_write_udp();
    }
}


// TCP ------------------------------------------------------

void AM::Network::m_do_read_tcp() {
    
    memset(m_tcprecv_data, 0, AM::MAX_PACKET_SIZE);

    m_tcp_socket.async_read_some(asio::buffer(m_tcprecv_data, AM::MAX_PACKET_SIZE),
            [this](std::error_code ec, std::size_t size) {
                if(ec) {
                    printf("[read_tcp](%i): %s\n", ec.value(), ec.message().c_str());
                    return;
                }

                m_handle_tcp_packet(size);
                m_do_read_tcp();
            });

}


void AM::Network::m_do_write_tcp() {
    if(!m_tcp_data_ready_to_send) {
        return;
    }
    m_tcp_data_ready_to_send = false;

    asio::async_write(m_tcp_socket, 
            asio::buffer(this->packet.data, this->packet.size),
            [this](std::error_code ec, std::size_t /*size*/) {
                if(ec) {
                    printf("[write_tcp](%i): %s\n", ec.value(), ec.message().c_str());
                    return;
                }               
            });
}




// UDP ------------------------------------------------------


void AM::Network::m_do_read_udp() {

    memset(m_udprecv_data, 0, AM::MAX_PACKET_SIZE);
    m_udp_socket.async_receive_from(
            asio::buffer(m_udprecv_data, AM::MAX_PACKET_SIZE), m_udp_sender_endpoint,
            [this](std::error_code ec, std::size_t size) {
                if(ec) {
                    printf("[read_udp](%i): %s\n", ec.value(), ec.message().c_str());
                    return;
                }

                m_handle_udp_packet(size);
                m_do_read_udp(); 
            });
}

void AM::Network::m_do_write_udp() {
    if(!m_udp_data_ready_to_send) {
        return;
    }
    m_udp_data_ready_to_send = false;

    m_udp_socket.async_send_to(
            asio::buffer(this->packet.data, this->packet.size), m_udp_sender_endpoint,
            [this](std::error_code ec, std::size_t) {
                if(ec) {
                    printf("[write_udp](%i): %s\n", ec.value(), ec.message().c_str());
                    return;
                }
            });
}


