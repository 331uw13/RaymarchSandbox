#ifndef AMBIENT3D_TERRAIN_HPP
#define AMBIENT3D_TERRAIN_HPP

#include <vector>
#include <cstdint>
#include <mutex>
#include <thread>
#include <atomic>
#include <map>

#include "raylib.h"

#include "chunk.hpp"
#include "ivector.hpp"


namespace AM {

    class Player;

    // TODO: These are not needed actually.
    static constexpr uint16_t MAX_TERRAIN_ROWS = 8;
    static constexpr uint16_t MAX_TERRAIN_CHUNKS = MAX_TERRAIN_ROWS * MAX_TERRAIN_ROWS;
    static constexpr float    TERRAIN_MAX_HEIGHT = 500.0f;

    class Terrain {
        public:
           
            std::map<IVector2, Chunk>  chunks;
            std::mutex                 chunks_mutex;
            


            void start_generator();
            void stop_generator();
            void unload_all_chunks();
            void find_new_chunks(int chunk_x, int chunk_z, int radius);
            void render(const Player& player, const Material& material, float radius);
            float get_height(float x, float z, Vector3* normal_out = NULL);

        private:
            std::atomic<bool>  m_threads_running;
            void               m_chunk_gen_th__func();
            std::thread        m_chunk_gen_th;

            std::vector<IVector2>     m_chunkpos_queue;

            void m_get_chunk_mesh_quad(
                    int chunk_x, int chunk_z,
                    int local_x, int local_z, Triangle2X* out);


    };

};



#endif
