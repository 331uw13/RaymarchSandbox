
#include <iostream>
#include "item_manager.hpp"


void AM::ItemManager::update_lifetimes() {
}

void AM::ItemManager::update_queue() {
    m_items_queue_mutex.lock();
    for(size_t i = 0; i < m_items_queue.size(); i++) {
        AM::Item* item = &m_items_queue[i];



    }

    m_items_queue.clear();
    m_items_queue_mutex.unlock();
}

void AM::ItemManager::add_itembase_to_queue(const AM::ItemBase& itembase) {
    m_items_queue_mutex.lock();
    m_items_queue.push_back((AM::Item)itembase);
    m_items_queue_mutex.unlock();
}


