#include <cstdlib>
#include <iostream>

#include "item_base.hpp"




void AM::ItemBase::load_info(const json& item_list, AM::ItemID item_id, const char* entry_name) {
    const size_t entry_name_size = strlen(entry_name);
    if(entry_name_size >= AM::ITEM_MAX_ENTRYNAME_SIZE) {
        fprintf(stderr, "ERROR! %s: Item entry name is too long (%s)\n",
                __func__, entry_name);
        return;
    }
    this->max_stack = 1;
    this->pos_x = 0;
    this->pos_y = 0;
    this->pos_z = 0;
    this->lifetime_ticks = 0;
    this->id = item_id;
    this->uuid = std::rand();

    const json& item_json = item_list[entry_name];

    std::string category = item_json["category"].template get<std::string>();
    
    std::string display_name   = item_json["display_name"].template get<std::string>();
    std::string model_path     = item_json["model_path"].template get<std::string>();
    std::string description    = item_json["description"].template get<std::string>();

    if(display_name.size() >= AM::ITEM_MAX_DISPLAYNAME_SIZE) {
        fprintf(stderr, "ERROR! %s: Item display name is too long (%s)\n",
                __func__, entry_name);
        return;
    } 
    if(description.size() >= AM::ITEM_MAX_DESC_SIZE) {
        fprintf(stderr, "ERROR! %s: Item description is too long (%s)\n",
                __func__, entry_name);
        return;
    }
    if(model_path.size() >= AM::ITEM_MAX_MODELPATH_SIZE) {
        fprintf(stderr, "ERROR! %s: Item model path is too long (%s)\n",
                __func__, entry_name);
        return;
    }

    memmove(this->entry_name, entry_name, entry_name_size);
    memmove(this->display_name, &display_name[0], display_name.size());
    memmove(this->description, &description[0], description.size());
    memmove(this->model_path, &model_path[0], model_path.size());


    if(category == "FOOD") { 
        this->type = AM::ItemType::FOOD;
        this->max_stack          = item_json["max_stack"].template get<int>();
        this->food.eat_benefit   = item_json["eat_benefit"].template get<float>();
        this->food.eat_duration  = item_json["eat_duration"].template get<float>();
    }
    else
    if(category == "TOOL") {
        this->type = AM::ItemType::TOOL;
        this->max_stack          = item_json["max_stack"].template get<int>();
        this->tool.max_usage     = item_json["max_usage"].template get<int>();
        this->tool.usage_cost    = item_json["usage_cost"].template get<int>();
        this->tool.usage         = 0;
    }
    else
    if(category == "WEAPON") {
        this->type = AM::ItemType::WEAPON;
        this->weapon.accuracy     = item_json["accuracy"].template get<float>();
        this->weapon.base_damage  = item_json["base_damage"].template get<float>();
        this->weapon.recoil       = item_json["recoil"].template get<float>();
        this->weapon.firerate     = item_json["firerate"].template get<float>();

        json firemode_array = item_json["firemode"];
        std::string firemode_name;
        for(const json& j : firemode_array) {
            firemode_name = j.template get<std::string>();
            if(firemode_name == "SEMI_AUTO") {
                this->weapon.firemode |= AM::FireMode::SEMI_AUTO;
            }
            else
            if(firemode_name == "FULL_AUTO") {
                this->weapon.firemode |= AM::FireMode::FULL_AUTO;
            } 
        }
    }


}



