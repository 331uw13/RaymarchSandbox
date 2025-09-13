#ifndef AMBIENT3D_NETWORKING_AGREEMENTS_HPP
#define AMBIENT3D_NETWORKING_AGREEMENTS_HPP

#include <cstdint>


namespace AM {
    
    static constexpr uint8_t PACKET_DATA_SEPARATOR = 0x1F;
    static constexpr uint8_t PACKET_DATA_STOP = 0x3;
    static constexpr size_t MAX_PACKET_SIZE = 1024;

};

#endif
