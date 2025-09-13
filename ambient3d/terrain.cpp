#include "terrain.hpp"
#include "player.hpp"

#include "raymath.h"
            

void AM::Terrain::start_generator() {
    m_threads_running = true;
    m_chunk_gen_th = std::thread(&AM::Terrain::m_chunk_gen_th__func, this);
}

void AM::Terrain::stop_generator() {
    m_threads_running = false;
    m_chunk_gen_th.join();
}

void AM::Terrain::unload_all_chunks() {
    SetTraceLogLevel(LOG_NONE);
    for(auto& [pos, chunk] : this->chunks) {
        chunk.unload();
    }
    SetTraceLogLevel(LOG_ALL);
}

void AM::Terrain::m_chunk_gen_th__func() {
    while(m_threads_running) {
        this->chunks_mutex.lock();
        for(size_t i = 0; i < m_chunkpos_queue.size(); i++) {
            const IVector2& pos = m_chunkpos_queue[i];              
            
            auto search = this->chunks.find(pos);
            if((search == this->chunks.end()) || !search->second.is_loaded()) {
                Chunk chunk;
                chunk.load(pos.x, pos.y);
                this->chunks.insert(std::make_pair(pos, chunk));
            }

            m_chunkpos_queue.erase(m_chunkpos_queue.begin()+i);
            i--;
        }

        this->chunks_mutex.unlock();
        /*
        std::this_thread::sleep_for(
                std::chrono::milliseconds(5));
                */
    }
}

void AM::Terrain::find_new_chunks(int chunk_x, int chunk_z, int radius) {
    SetTraceLogLevel(LOG_NONE);
    this->chunks_mutex.lock();
    for(std::map<IVector2, AM::Chunk>::iterator it
            = this->chunks.begin(); it != this->chunks.end(); it++) {
        if(it->second.distance(chunk_x, chunk_z) < 32) {
            continue;
        }
        it->second.unload();
        it = this->chunks.erase(it);
    }
    for(int z = -radius; z <= radius; z++) {
        for(int x = -radius; x <= radius; x++) {
            const IVector2 pos(x+chunk_x, z+chunk_z);
            m_chunkpos_queue.push_back(pos);
        }
    }
    
    this->chunks_mutex.unlock();
    SetTraceLogLevel(LOG_ALL);
}
            
void AM::Terrain::render(const Player& player, const Material& material, float radius) {
    SetTraceLogLevel(LOG_NONE);
    this->chunks_mutex.lock();
    
    for(auto& [ pos, chunk ] : this->chunks) {
        if(!chunk.is_loaded()) {
            continue;
        }
        float dist = chunk.distance(player.chunk_x, player.chunk_z);
        if(dist > radius) {
            continue;
        }
        if(!chunk.is_uploaded()) {
            chunk.upload();
        }
        chunk.render(material, 0.0f);
    }

    this->chunks_mutex.unlock();
    SetTraceLogLevel(LOG_ALL);
}


// IMPORTANT NOTE: chunks_mutex must be locked before calling this!
void AM::Terrain::m_get_chunk_mesh_quad(
        int chunk_x, int chunk_z,
        int local_x, int local_z, Triangle2X* out) {

    // Set coordinates to neighbor chunks if would overflow.
    if(local_x < 0) {
        local_x = AM::CHUNK_SIZE-1;
        chunk_x--;
    }
    else
    if(local_x >= AM::CHUNK_SIZE) {
        local_x = 0;
        chunk_x++;
    }
    if(local_z < 0) {
        local_z = AM::CHUNK_SIZE-1;
        chunk_z--;
    }
    else
    if(local_z >= AM::CHUNK_SIZE) {
        local_z = 0;
        chunk_z++;
    }

    const auto search = this->chunks.find(IVector2(chunk_x, chunk_z));
    if(search != this->chunks.end()) {
        if(!search->second.is_loaded()) {
            return;
        }
        *out = search->second.trlookup_table[local_z * AM::CHUNK_SIZE + local_x];
        Vector3 p = Vector3(
                chunk_x * (AM::CHUNK_SIZE * AM::CHUNK_SCALE),
                0,
                chunk_z * (AM::CHUNK_SIZE * AM::CHUNK_SCALE));
        
        out->a0 += p;
        out->a1 += p;
        out->a2 += p;
        out->b0 += p;
        out->b1 += p;
        out->b2 += p;
    }
}

float AM::Terrain::get_height(float x, float z, Vector3* normal_out) {
    this->chunks_mutex.lock();

    int chunk_x = (int)floor(x / (AM::CHUNK_SIZE * AM::CHUNK_SCALE));
    int chunk_z = (int)floor(z / (AM::CHUNK_SIZE * AM::CHUNK_SCALE));

    int rx = (int)floor(x / AM::CHUNK_SCALE);
    int rz = (int)floor(z / AM::CHUNK_SCALE);

    // Convert x, y to local chunk coordinates.
    int local_x = (rx % AM::CHUNK_SIZE + AM::CHUNK_SIZE) % AM::CHUNK_SIZE;
    int local_z = (rz % AM::CHUNK_SIZE + AM::CHUNK_SIZE) % AM::CHUNK_SIZE;

    RayCollision hit_info;
    Ray ray = {
        .position = Vector3(x, AM::TERRAIN_MAX_HEIGHT, z),
        .direction = Vector3(0.0f, -1.0f, 0.0f)
    };

    Triangle2X quad;

    for(int iz = -1; iz <= 1; iz++) {
        for(int ix = -1; ix <= 1; ix++) {
            m_get_chunk_mesh_quad(
                            chunk_x,
                            chunk_z,
                            local_x + ix,
                            local_z + iz,
                            &quad);

            hit_info = GetRayCollisionTriangle(ray, quad.a0, quad.a1, quad.a2);
            if(hit_info.hit) {
                goto loop_exit;
            }
 
            hit_info = GetRayCollisionTriangle(ray, quad.b0, quad.b1, quad.b2);
            if(hit_info.hit) {
                goto loop_exit;
            }
        }
    }
loop_exit:

    if(normal_out && hit_info.hit) {
        *normal_out = hit_info.normal;
    }

    this->chunks_mutex.unlock();
    return (hit_info.hit) ? (AM::TERRAIN_MAX_HEIGHT - hit_info.distance) : 0;
}

