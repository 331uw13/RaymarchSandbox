#ifndef AMBIENT3D_PACKET_WRITER_HPP
#define AMBIENT3D_PACKET_WRITER_HPP

#include <cstddef>
#include <string>
#include <initializer_list>

#include "networking_agreements.hpp"
#include "packet_ids.hpp"


namespace AM {
 
    enum PacketStatus : int {
        NOT_PREPARED=0, // Writing is not allowed because packet id would be missing.
        PREPARED,  // Prepared but not written anything yet.
        HAS_DATA,  // Prepared and has data.
    };

    struct Packet {
        char   data[AM::MAX_PACKET_SIZE] { 0 };
        size_t size { 0 };
        
        PacketStatus status;
    };

    // These functions help writing the packet data as bytes
    // it can then be sent either on client side with AM::Network
    // or at server side AM::TCP_session or AM::UDP_handler.

    // NOTE: writing will fail silently if the packet is not prepared.

    void packet_prepare         (Packet* packet, AM::PacketID packet_id);
    void packet_write_string    (Packet* packet, const std::string& str);
    void packet_write_int       (Packet* packet, std::initializer_list<int> list);
    void packet_write_float     (Packet* packet, std::initializer_list<float> list);
    void packet_write_separator (Packet* packet);
};






#endif
