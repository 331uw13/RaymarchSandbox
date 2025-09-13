
#include <cstdlib>
#include "chunk.hpp"
#include "perlin_noise.hpp"
#include "terrain.hpp"

#include "raymath.h"


#include <cstdio>


static float _compute_noise_level(float X, float Z) { 
    float level 
        = perlin_noise_2D(X, Z) * 3.0
        + perlin_noise_2D(X*0.1, Z*0.1)*20.0
        + ((perlin_noise_2D(X*0.2, Z*0.2)*120.0)
        * perlin_noise_2D(X*0.05, Z*0.05))
        + ((perlin_noise_2D(X*0.05, Z*0.05)*500.0)
        * perlin_noise_2D(X*0.01, Z*0.01));
 
    return level;
}

void AM::Chunk::m_generate_heightmap(int origin_x, int origin_z) {
    const float tsize = (float)AM::MAX_TERRAIN_ROWS;

    int index = 0;
    for(int z = 0; z <= CHUNK_SIZE; z++) {
        for(int x = 0; x <= CHUNK_SIZE; x++) {

            float X = (x + origin_x * (CHUNK_SIZE)) / tsize;
            float Z = (z + origin_z * (CHUNK_SIZE)) / tsize;

            m_heightmap[index] = _compute_noise_level(X, Z);
            index++;
        }
    }
}

float AM::Chunk::m_get_height_at_local(int x, int z) {
    int64_t idx = z * (AM::CHUNK_SIZE+1) + x;
    if(idx < 0) {
        return 0.0f;
    }
    if(idx >= (AM::CHUNK_SIZE+1)*(AM::CHUNK_SIZE+1)) {
        return 0.0f;
    }
    return m_heightmap[idx];
}


void AM::Chunk::load(int chunk_x, int chunk_z) {
    if(m_loaded) {
        fprintf(stderr, "WARNING! Trying to load already loaded chunk.\n");
        return;
    }

    m_mesh = Mesh{
        .vertexCount = 0,
        .triangleCount = 0,
        .vertices = NULL,
        .texcoords = NULL,
        .texcoords2 = NULL,
        .normals = NULL,
        .tangents = NULL,
        .colors = NULL,
        .indices = NULL,
        .animVertices = NULL,
        .animNormals = NULL,
        .boneIds = NULL,
        .boneWeights = NULL,
        .boneMatrices = NULL,
        .boneCount = 0,
        .vaoId = 0,
        .vboId = NULL
    };
    m_generate_heightmap(chunk_x, chunk_z);

    m_mesh.triangleCount = (AM::CHUNK_SIZE * AM::CHUNK_SIZE) * 2;
    m_mesh.vertexCount = m_mesh.triangleCount * 3;

    m_mesh.vertices = (float*)malloc(m_mesh.vertexCount * 3 * sizeof(float));
    m_mesh.normals = (float*)malloc(m_mesh.vertexCount * 3 * sizeof(float));
    m_mesh.texcoords = (float*)malloc(m_mesh.vertexCount * 2 * sizeof(float));

    // Triangle look up table is used to get a triangle with player's X, Z position.
    this->trlookup_table = (Triangle2X*)malloc(m_mesh.triangleCount * sizeof(Triangle2X));

    // Used for calculating normals.
    Vector3 vA(0, 0, 0);
    Vector3 vB(0, 0, 0);
    Vector3 vC(0, 0, 0);

    int v_counter = 0; // Count vertices.
    //int n_counter = 0; // Count normals.
    int tc_counter = 0; // Count texcoords.

    for(int z = 0; z < CHUNK_SIZE; z++) {
        for(int x = 0; x < CHUNK_SIZE; x++) {
       

            // Left up corner triangle.

            m_mesh.vertices[v_counter]   = (float)x * CHUNK_SCALE;
            m_mesh.vertices[v_counter+1] = m_get_height_at_local(x, z);
            m_mesh.vertices[v_counter+2] = (float)z * CHUNK_SCALE;

            m_mesh.vertices[v_counter+3] = (float)x * CHUNK_SCALE;
            m_mesh.vertices[v_counter+4] = m_get_height_at_local(x, z+1);
            m_mesh.vertices[v_counter+5] = (float)(z+1) * CHUNK_SCALE;

            m_mesh.vertices[v_counter+6] = (float)(x+1) * CHUNK_SCALE;
            m_mesh.vertices[v_counter+7] = m_get_height_at_local(x+1, z);
            m_mesh.vertices[v_counter+8] = (float)z * CHUNK_SCALE;

            // Right bottom corner triangle.

            m_mesh.vertices[v_counter+9] = m_mesh.vertices[v_counter+6];
            m_mesh.vertices[v_counter+10] = m_mesh.vertices[v_counter+7];
            m_mesh.vertices[v_counter+11] = m_mesh.vertices[v_counter+8];

            m_mesh.vertices[v_counter+12] = m_mesh.vertices[v_counter+3];
            m_mesh.vertices[v_counter+13] = m_mesh.vertices[v_counter+4];
            m_mesh.vertices[v_counter+14] = m_mesh.vertices[v_counter+5];

            m_mesh.vertices[v_counter+15] = (float)(x+1) * CHUNK_SCALE;
            m_mesh.vertices[v_counter+16] = m_get_height_at_local(x+1, z+1);
            m_mesh.vertices[v_counter+17] = (float)(z+1) * CHUNK_SCALE;


            this->trlookup_table[(z*CHUNK_SIZE+x)] = {
                Vector3(
                        m_mesh.vertices[v_counter],
                        m_mesh.vertices[v_counter+1],
                        m_mesh.vertices[v_counter+2]),
                Vector3(
                        m_mesh.vertices[v_counter+3],
                        m_mesh.vertices[v_counter+4],
                        m_mesh.vertices[v_counter+5]),
                Vector3(
                        m_mesh.vertices[v_counter+6],
                        m_mesh.vertices[v_counter+7],
                        m_mesh.vertices[v_counter+8]),
    
                Vector3(
                        m_mesh.vertices[v_counter+9],
                        m_mesh.vertices[v_counter+10],
                        m_mesh.vertices[v_counter+11]),
                Vector3(
                        m_mesh.vertices[v_counter+12],
                        m_mesh.vertices[v_counter+13],
                        m_mesh.vertices[v_counter+14]),
                Vector3(
                        m_mesh.vertices[v_counter+15],
                        m_mesh.vertices[v_counter+16],
                        m_mesh.vertices[v_counter+17])
            };

            v_counter += 18;

            // Texture coordinates.

            const float m_s = (float)8 - 1;
            const float tx = (float)x;
            const float tz = (float)z;

            m_mesh.texcoords[tc_counter]   = tx / m_s;
            m_mesh.texcoords[tc_counter+1] = tz / m_s;

            m_mesh.texcoords[tc_counter+2] = tx / m_s;
            m_mesh.texcoords[tc_counter+3] = (tz+1) / m_s;

            m_mesh.texcoords[tc_counter+4] = (tx+1) / m_s;
            m_mesh.texcoords[tc_counter+5] = tz / m_s;

            m_mesh.texcoords[tc_counter+6] = m_mesh.texcoords[tc_counter + 4];
            m_mesh.texcoords[tc_counter+7] = m_mesh.texcoords[tc_counter + 5];

            m_mesh.texcoords[tc_counter+8] = m_mesh.texcoords[tc_counter + 2];
            m_mesh.texcoords[tc_counter+9] = m_mesh.texcoords[tc_counter + 3];

            m_mesh.texcoords[tc_counter+10] = (tx+1) / m_s;
            m_mesh.texcoords[tc_counter+11] = (tz+1) / m_s;

            tc_counter += 12;
        }
    }

    // Calculate normals.
    for(int idx = 0; idx < m_mesh.vertexCount*3; idx+=9) {

        vA.x = m_mesh.vertices[idx + 0];
        vA.y = m_mesh.vertices[idx + 1];
        vA.z = m_mesh.vertices[idx + 2];
        
        vB.x = m_mesh.vertices[idx + 3];
        vB.y = m_mesh.vertices[idx + 4];
        vB.z = m_mesh.vertices[idx + 5];
        
        vC.x = m_mesh.vertices[idx + 6];
        vC.y = m_mesh.vertices[idx + 7];
        vC.z = m_mesh.vertices[idx + 8];

        Vector3 vN = Vector3Normalize(Vector3CrossProduct(
                    Vector3Subtract(vB, vA), Vector3Subtract(vC, vA)));
       
        for(int k = 0; k < 9; k += 3) { 
            m_mesh.normals[idx + 0 + k] = vN.x;
            m_mesh.normals[idx + 1 + k] = vN.y;
            m_mesh.normals[idx + 2 + k] = vN.z;
        }
    }
   




    m_chunk_x = chunk_x;
    m_chunk_z = chunk_z;
    m_loaded = true;
    m_uploaded = false;
}
 
void AM::Chunk::upload() {
    if(!m_loaded) {
        fprintf(stderr, "ERROR! chunk must be loaded before trying to upload it.\n");
        return;
    }
    if(m_uploaded) {
        return;
    }
    UploadMesh(&m_mesh, false);
    m_uploaded = true;
}

void AM::Chunk::unload() {
    if(!m_loaded) {
        fprintf(stderr, "WARNING! Trying to unload already unloaded chunk.\n");
        return;
    }
    UnloadMesh(m_mesh);
    m_mesh.vertices = NULL;
    m_mesh.normals = NULL;
    m_mesh.texcoords = NULL;
    free(this->trlookup_table);
    this->trlookup_table = NULL;
    m_loaded = false;
}


void AM::Chunk::render(const Material& material, float ylevel) {
    if(!m_loaded) {
        return;
    }
    if(!m_uploaded) {
        return;
    }
    DrawMesh(m_mesh, material, MatrixTranslate(
                (float)m_chunk_x * (AM::CHUNK_SIZE * AM::CHUNK_SCALE),
                ylevel,
                (float)m_chunk_z * (AM::CHUNK_SIZE * AM::CHUNK_SCALE)
                ));
}

float AM::Chunk::distance(int chunk_x, int chunk_z) {
    return Vector2Distance(Vector2(chunk_x, chunk_z), Vector2(m_chunk_x, m_chunk_z));
}


