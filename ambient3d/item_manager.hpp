#ifndef AMBIENT3D_ITEM_MANAGER_HPP
#define AMBIENT3D_ITEM_MANAGER_HPP


// Item manager handles loading and unloading received items.
// Their 3D models, inventory textures and item type specific information.


#include <map>
#include <array>
#include <nlohmann/json.hpp>
#include <deque>
#include <mutex>

#include "raylib.h"
#include "item.hpp"

using json = nlohmann::json;


namespace AM {

    class ItemManager {
        public:

            void update_lifetimes();
            void update_queue();
            void add_itembase_to_queue(const AM::ItemBase& itembase);



        private:

            std::mutex             m_items_queue_mutex;
            std::deque<AM::Item>   m_items_queue;


    };


};


#endif
