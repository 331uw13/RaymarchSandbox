#ifndef AMBIENT3D_CHUNK_HPP
#define AMBIENT3D_CHUNK_HPP

#include <cstdint>
#include <array>

#include "raylib.h"
#include "ivector.hpp"


// TODO: Add Level of detail...


namespace AM {

    struct Triangle2X {
        Vector3 a0;
        Vector3 a1;
        Vector3 a2;
        Vector3 b0;
        Vector3 b1;
        Vector3 b2;
    };

    static constexpr uint16_t   CHUNK_SIZE = 32;
    static constexpr size_t HEIGHTMAP_SIZE = (CHUNK_SIZE+1)*(CHUNK_SIZE+1);
    static constexpr float CHUNK_SCALE = 2.0f;

    class Chunk {
        public:

            Chunk() {
                m_loaded = false;
            }
            ~Chunk() {}
            
            Triangle2X* trlookup_table;


            //==== Public Functions====
            bool is_loaded() { return m_loaded; }
            bool is_uploaded() { return m_uploaded; }
            void load(int chunk_x, int chunk_z);
            void upload(); // Upload mesh to vram. Note: Must be called from main thread.
            void unload();
            void render(const Material& material, float ylevel);

            IVector2 get_grid_pos() { return IVector2(m_chunk_x, m_chunk_z); }

            float distance(int chunk_x, int chunk_z);

        private:

            //==== Private Functions ====
            void m_generate_heightmap(int origin_x, int origin_z);
            float m_get_height_at_local(int x, int z);

            //==== Private Variables ====
            std::array<float, HEIGHTMAP_SIZE> m_heightmap;
            Mesh m_mesh;
            int m_chunk_x;
            int m_chunk_z;
            bool m_loaded;
            bool m_uploaded;
    };


};



#endif
