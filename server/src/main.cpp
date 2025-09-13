#include <cstdio>
#include <cstdlib>

#include "server.hpp"


int main() {
    printf("Ambient3D - Server\n");

    asio::io_context io_context;

    const uint16_t tcp_port = 34480;
    const uint16_t udp_port = 34485;
    const char*    item_list_path = "../items/item_list.json";

    printf("Listening on port (tcp = %i) | (udp = %i)\n",
            tcp_port, udp_port);

    AM::Server server(io_context, tcp_port, udp_port);
    if(server.parse_item_list(item_list_path)) {
        server.start(io_context);
    }
}

