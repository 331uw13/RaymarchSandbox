
#include <cstdio>
#include <iostream>
#include <chrono>
#include <ctime>
#include <cstdlib>

#include <fstream>
#include <iostream>

#include "server.hpp"


        
AM::Server::~Server() {

    printf("Server closed.\n");
}


void AM::Server::start(asio::io_context& io_context) {

    m_udp_handler.start(this);
    m_do_accept_TCP();

    // Set random seed to current time.
    std::srand(std::time({}));

    // Start the event handler.
    std::thread event_handler([](asio::io_context& context) {
        context.run();
    }, std::ref(io_context));


    // Start user input handler.
    m_userinput_handler_th = 
        std::thread(&AM::Server::m_userinput_handler_th__func, this);

    // Start the updater loop.
    m_update_loop_th =
        std::thread(&AM::Server::m_update_loop_th__func, this);

    // Spawn items for testing
    this->spawn_item(AM::ItemID::M4A16, 1, Vec3{ 3, 5, -2 });
    this->spawn_item(AM::ItemID::HEAVY_AXE, 1, Vec3{ 6, 5, -2 });

    // Input handler may tell to shutdown.
    m_userinput_handler_th.join();

    io_context.stop();
    event_handler.join();
    m_update_loop_th.join();
}

            
void AM::Server::remove_player(int player_id) {
    this->players_mutex.lock();

    const auto search = this->players.find(player_id);
    if(search != this->players.end()) {
        this->players.erase(search);
    }

    this->players_mutex.unlock();

    constexpr size_t msgbuf_size = 512;
    char msgbuf[msgbuf_size] = { 0 };
    snprintf(msgbuf, msgbuf_size-1, "Player %i left the server", player_id);
    this->broadcast_msg(PacketID::SERVER_MESSAGE, msgbuf);
}

void AM::Server::m_do_accept_TCP() {
    
    // Context will call this lambda when connection arrives.
    m_tcp_acceptor.async_accept(
            [this](std::error_code ec, tcp::socket socket) {
                if(ec) {
                    printf("[accept](%i): %s\n", ec.value(), ec.message().c_str());
                
                    m_do_accept_TCP();
                    return;
                }

                this->players_mutex.lock();

                int player_id = m_next_player_id++; // <- TODO: MAKE SURE THIS WILL NEVER OVERFLOW!
                Player player(std::make_shared<TCP_session>(std::move(socket), this, player_id));
                player.id = player_id;

                player.tcp_session->start();
                this->players.insert(std::make_pair(player_id, player));
                this->players_mutex.unlock();

                printf("[+] Player joined! ID = %i\n", player_id);

                // Send player id.
                AM::packet_prepare  (&player.tcp_session->packet, AM::PacketID::PLAYER_ID);
                AM::packet_write_int(&player.tcp_session->packet, { player_id });
                player.tcp_session->send_packet();

                // Send item list.
                AM::packet_prepare(&player.tcp_session->packet, AM::PacketID::SAVE_ITEM_LIST);
                AM::packet_write_string(&player.tcp_session->packet, m_item_list.dump());
                player.tcp_session->send_packet();

                m_do_accept_TCP();
            });

}
            
AM::Player* AM::Server::get_player_by_id(int player_id) {
    this->players_mutex.lock();
    const auto search = this->players.find(player_id);
    if(search == this->players.end()) {
        fprintf(stderr, "ERROR! No player found with ID: %i\n", player_id);
        this->players_mutex.unlock();
        return NULL;
    }

    this->players_mutex.unlock();
    return (AM::Player*)&search->second;
}


void AM::Server::broadcast_msg(AM::PacketID packet_id, const std::string& msg) {
    this->players_mutex.lock();

    for(auto it = this->players.begin(); it != this->players.end(); ++it) {
        Player* p = &it->second;
        AM::packet_prepare(&p->tcp_session->packet, packet_id);
        AM::packet_write_string(&p->tcp_session->packet, msg);
        p->tcp_session->send_packet();
    }

    this->players_mutex.unlock();
}


void AM::Server::m_update_players() {
    this->players_mutex.lock();

    // Tell players each others position, camera yaw and pitch.
    
    // TODO: Wallhacks, "ESP" cheats can be prevented
    // by only telling the player's position when they can see them
    // NOTE to self: think about ways how to bypass this.

    for(auto itA = this->players.begin();
            itA != this->players.end(); ++itA) {
        const Player* player = &itA->second;

        for(auto itB = this->players.begin();
                itB != this->players.end(); ++itB) {
            const Player* p = &itB->second;
            if(p->id == player->id) { continue; }

            AM::packet_prepare(&m_udp_handler.packet, AM::PacketID::PLAYER_MOVEMENT_AND_CAMERA);
            AM::packet_write_int(&m_udp_handler.packet, { player->id });
            AM::packet_write_float(&m_udp_handler.packet, {
                        player->pos.x,
                        player->pos.y,
                        player->pos.z,
                        player->cam_yaw,
                        player->cam_pitch
                    });
            AM::packet_write_int(&m_udp_handler.packet, { player->anim_id });

            m_udp_handler.send_packet(p->id);
        }
    }

    this->players_mutex.unlock();
}

void AM::Server::m_update_items() {
    if(this->dropped_items.empty()) {
        return;
    }
    
    this->dropped_items_mutex.lock();

    // Loop through all players online, collect and send nearby item info.

    for(auto it = this->players.begin();
            it != this->players.end(); ++it) {
        const Player* player = &it->second;

        AM::packet_prepare(&m_udp_handler.packet, AM::PacketID::ITEM_UPDATE);

        for(size_t i = 0; i < this->dropped_items.size(); i++) {
            AM::ItemBase* item = &this->dropped_items[i];

            AM::packet_write_int(&m_udp_handler.packet, {
                    item->uuid,
                    item->id
            });
            AM::packet_write_float(&m_udp_handler.packet, {
                    item->pos_x,
                    item->pos_y,
                    item->pos_z
            });
            AM::packet_write_string(&m_udp_handler.packet, item->entry_name);
            AM::packet_write_separator(&m_udp_handler.packet);
        }

        m_udp_handler.send_packet(player->id);
    }

    this->dropped_items_mutex.unlock();
}

void AM::Server::m_update_loop_th__func() {
    while(m_keep_threads_alive) {
       
        m_update_players();
        m_update_items();

        std::this_thread::sleep_for(
                std::chrono::milliseconds(TICK_SPEED_MS));
    }
}


void AM::Server::m_userinput_handler_th__func() {
    std::string input;
    input.reserve(256);

    while(m_keep_threads_alive) {

        std::cin >> input;
        if(input.empty()) { continue; }

        // TODO: Create better system for commands.

        if(input == "end") {
            m_keep_threads_alive = false;
        }
        else
        if(input == "clear") {
            printf("\033[2J\033[H");
            fflush(stdout);
        }
        else
        if(input == "spawn_item") {
            this->spawn_item(AM::ItemID::M4A16, 1, Vec3{ 3, 5, -2 });
        }
        else 
        if(input == "show_debug") {
            this->show_debug_info = true;
        }
        else
        if(input == "hide_debug") {
            this->show_debug_info = false;
        }
        else
        if(input == "online") {
            this->players_mutex.lock();
            printf("Online players: %li\n", this->players.size());
            this->players_mutex.unlock();
        }
        else {
            printf(" Unknown command.\n");
        }

    }

    // NOTE: When the above while loop is exited.
    //       The server will shutdown.
}
            
            
bool AM::Server::parse_item_list(const char* item_list_path) {
    std::fstream item_list_stream(item_list_path);
    if(!item_list_stream.is_open()) {
        fprintf(stderr, "ERROR! %s: Failed to open item list (%s)\n",
                __func__, item_list_path);
        return false;
    }

    m_item_list = json::parse(item_list_stream);

    this->load_item("apple", AM::ItemID::APPLE);
    this->load_item("assault_rifle_A", AM::ItemID::M4A16);
    this->load_item("heavy_axe", AM::ItemID::HEAVY_AXE);


    return true;
}

void AM::Server::load_item(const char* entry_name, AM::ItemID item_id) {
    std::cout 
        << __func__
        << "(\""
        << entry_name
        << "\") -> " << m_item_list[entry_name].dump(4) << std::endl;
    
    this->items[item_id].load_info(m_item_list, item_id, entry_name);

    /*
    AM::ItemBase item;
    item.max_stack = 1;
    item.pos_x = 0;
    item.pos_y = 0;
    item.pos_z = 0;
    item.lifetime_ticks = 0;
    item.id = item_id;
    item.uuid = 0;
    std::string display_name = m_item_list[entry_name]["display_name"].template get<std::string>();
    std::string description = m_item_list[entry_name]["description"].template get<std::string>();

    // Load general item data first.

    const size_t entry_name_size = strlen(entry_name);
    if(entry_name_size >= AM::ITEM_MAX_ENTRY_NAME_SIZE) {
        fprintf(stderr, "ERROR! %s: entry_name (%s) is too long.\n",
                __func__, entry_name);
        return;
    }

    memmove(item.entry_name,    entry_name,       entry_name_size);
    //memmove(item.display_name,  &display_name[0], display_name.size());
    //memmove(item.desc,          &description[0],  description.size());
    
    const std::string category = m_item_list[entry_name]["category"].template get<std::string>();

    // Load category specific item data.

    if(category == "FOOD") {
        item.type = AM::ItemType::FOOD;
        item.food.eat_benefit  = m_item_list[entry_name]["eat_benefit"].template get<int>();
        item.max_stack         = m_item_list[entry_name]["max_stack"].template get<int>();
    } 
    else
    if(category == "TOOL") {
        item.type = AM::ItemType::TOOL;
        item.tool.max_usage  = m_item_list[entry_name]["max_usage"].template get<int>();
        item.tool.usage_cost  = m_item_list[entry_name]["usage_cost"].template get<int>();
        item.tool.usage = 0;
        item.max_stack         = m_item_list[entry_name]["max_stack"].template get<int>();
    }
    else
    if(category == "WEAPON") {
        item.type = AM::ItemType::WEAPON;
        item.weapon.accuracy     = m_item_list[entry_name]["accuracy"].template get<float>();
        item.weapon.base_damage  = m_item_list[entry_name]["base_damage"].template get<float>();
        item.weapon.recoil       = m_item_list[entry_name]["recoil"].template get<float>();
        item.weapon.firerate     = m_item_list[entry_name]["firerate"].template get<float>();
        item.weapon.firemode     = 0;
        json firemode = m_item_list[entry_name]["firemode"];

        std::string firemode_name;
        for(const json& j : firemode) {
            firemode_name = j.template get<std::string>();
            if(firemode_name == "SEMI_AUTO") {
                item.weapon.firemode |= AM::FireMode::SEMI_AUTO;
            }
            else
            if(firemode_name == "FULL_AUTO") {
                item.weapon.firemode |= AM::FireMode::FULL_AUTO;
            }
        }
    }
    else {
        fprintf(stderr, "ERROR! %s: \"%s\" is missing category.\n",
                __func__, entry_name);
        return;
    }
    */

}


// TODO: Probably good idea to move chunk generation to server side.
void AM::Server::spawn_item(AM::ItemID item_id, int count, const Vec3& pos) {
    if(item_id >= AM::ItemID::NUM_ITEMS) {
        fprintf(stderr, "ERROR! %s: Invalid item_id\n", __func__);
        return;
    }
    
    this->dropped_items_mutex.lock();
        
    this->dropped_items.push_back(this->items[item_id]);
    AM::ItemBase& item = this->dropped_items.back();
    item.pos_x = pos.x;
    item.pos_y = pos.y;
    item.pos_z = pos.z;
    item.lifetime_ticks = 0;
    item.uuid = std::rand();

    printf("%s -> \"%s\" XYZ = (%0.1f, %0.1f, %0.1f) UUID = %i\n", 
            __func__, 
            item.entry_name,
            pos.x,
            pos.y,
            pos.z,
            item.uuid);
    
    this->dropped_items_mutex.unlock();
}

