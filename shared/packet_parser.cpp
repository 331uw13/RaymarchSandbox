#include <cstring>
#include <cstdio>

#include "packet_parser.hpp"
    


AM::PacketID AM::parse_network_packet(char* buffer, size_t& sizeb) {
    AM::PacketID packet_id = AM::PacketID::NONE;

    if(sizeb < sizeof(AM::PacketID)) {
        fprintf(stderr, "ERROR! Received packet cannot be valid because its less than %li bytes. Ignoring...\n",
                sizeof(AM::PacketID));
        return packet_id;
    }

    // The packet id is the first 4 bytes.
    memmove(&packet_id, buffer, sizeof(AM::PacketID));

    // If the packet contains more data than
    // just the id. Move it to the beginning.
    if(sizeb > sizeof(AM::PacketID)) {
        memmove(buffer,
                buffer + sizeof(AM::PacketID),
                sizeb - sizeof(AM::PacketID));
        // The data is now moved to the beginning. Set remaining (now garbage) to zero.
        memset(buffer + (sizeb - sizeof(AM::PacketID)), 0, sizeof(AM::PacketID));
        sizeb -= sizeof(AM::PacketID);
    }

    return packet_id;
}




