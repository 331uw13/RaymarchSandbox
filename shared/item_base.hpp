#ifndef AMBIENT3D_ITEM_BASE_HPP
#define AMBIENT3D_ITEM_BASE_HPP

#include <string>
#include <cstdint>
#include <nlohmann/json.hpp>

#include "items/weapon.hpp"
#include "items/tool.hpp"
#include "items/food.hpp"


/*
   How items work with clients and server:
   Server will tell the players who are near item its "entry_name".
   The 'entry_name' is used to know item settings
   from item/item_list.json file.
   The file is sent by the server when client connects.

   This way adding and modifying items even at runtime is easy.
   And clients have more control over memory usage.
*/

using json = nlohmann::json;

namespace AM {

    static constexpr size_t ITEM_MAX_ENTRYNAME_SIZE = 31;
    static constexpr size_t ITEM_MAX_DISPLAYNAME_SIZE = 15;
    static constexpr size_t ITEM_MAX_DESC_SIZE = 47;
    static constexpr size_t ITEM_MAX_MODELPATH_SIZE = 47;

    enum ItemID : int {
        APPLE=0,
        M4A16,
        HEAVY_AXE,

        NUM_ITEMS
    };

    enum ItemType : int {
        FOOD=1,
        WEAPON,
        TOOL
    };

    class ItemBase {
        public:
            void load_info(const json& item_list, AM::ItemID item_id, const char* entry_name);

            char      entry_name    [ITEM_MAX_ENTRYNAME_SIZE+1] { 0 };
            char      display_name  [ITEM_MAX_DISPLAYNAME_SIZE+1] { 0 };
            char      description   [ITEM_MAX_DESC_SIZE+1] { 0 };
            char      model_path    [ITEM_MAX_MODELPATH_SIZE+1] { 0 };
            uint8_t   max_stack;
            int       uuid;

            float pos_x;
            float pos_y;
            float pos_z;

            int   lifetime_ticks; // (only used by server)
            
            ItemType  type;
            ItemID    id;

            union {
                AM::WeaponStruct  weapon;
                AM::FoodStruct    food;
                AM::ToolStruct    tool;
            };

        private:

    };


};


#endif
