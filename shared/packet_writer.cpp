#include <cstdio>
#include <cstring>

#include "packet_writer.hpp"


static bool _write_bytes(AM::Packet* packet, void* data, size_t data_sizeb) { 
    
    if(packet->size + data_sizeb >= AM::MAX_PACKET_SIZE) {
        fprintf(stderr, "ERROR! %s: Packet cant hold more data than %li bytes.\n",
                __func__, AM::MAX_PACKET_SIZE);
        return false;
    }

    memmove(packet->data + packet->size,
            data,
            data_sizeb);
    packet->size += data_sizeb;

    return true;
}

void AM::packet_prepare(AM::Packet* packet, AM::PacketID packet_id) {
    memset(packet->data, 0, AM::MAX_PACKET_SIZE);
    packet->size = 0;
    memmove(packet->data, &packet_id, sizeof(AM::PacketID));
    packet->size += sizeof(AM::PacketID);
    packet->status = AM::PacketStatus::PREPARED;
}

void AM::packet_write_string(AM::Packet* packet, const std::string& str) {
    if(packet->status == AM::PacketStatus::NOT_PREPARED) { return; }
    if(!_write_bytes(packet, (void*)&str[0], str.size())) {
        return;
    }
    packet->status = AM::PacketStatus::HAS_DATA;
}

void AM::packet_write_int(AM::Packet* packet, std::initializer_list<int> list) {
    if(packet->status == AM::PacketStatus::NOT_PREPARED) { return; }
    for(auto it = list.begin(); it != list.end(); ++it) {
        if(!_write_bytes(packet, (void*)it, sizeof(int))) {
            return;
        }
    }
    packet->status = AM::PacketStatus::HAS_DATA;
}

void AM::packet_write_float(AM::Packet* packet, std::initializer_list<float> list) {
    if(packet->status == AM::PacketStatus::NOT_PREPARED) { return; }
    for(auto it = list.begin(); it != list.end(); ++it) {
        if(!_write_bytes(packet, (void*)it, sizeof(float))) {
            return;
        }
    }
    packet->status = AM::PacketStatus::HAS_DATA;
}

void AM::packet_write_separator(AM::Packet* packet) {
    if(packet->status == AM::PacketStatus::NOT_PREPARED) { return; }
    if(!_write_bytes(packet, (void*)&AM::PACKET_DATA_SEPARATOR, sizeof(AM::PACKET_DATA_SEPARATOR))) {
        return;
    }
    packet->status = AM::PacketStatus::HAS_DATA;
}


