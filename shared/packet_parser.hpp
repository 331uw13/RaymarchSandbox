#ifndef AMBIENT3D_NETWORK_PACKET_PARSER_HPP
#define AMBIENT3D_NETWORK_PACKET_PARSER_HPP


#include "packet_ids.hpp"


namespace AM {

    // This function is a helper for TCP and UDP packet handling.
    // It takes the received data as input and modifies it
    // so that it returns the packet id and removes the sizeof(AM::PacketID) 
    // number of bytes from the beginning.

    AM::PacketID parse_network_packet(char* buffer, size_t& sizeb);


};






#endif
