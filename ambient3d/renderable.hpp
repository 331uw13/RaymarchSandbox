#ifndef AMBIENT3D_RENDERABLE_OBJECT_HPP
#define AMBIENT3D_RENDERABLE_OBJECT_HPP


#include <vector>
#include <cstdint>

#include "raylib.h"

#include "animation.hpp"


namespace AM {

    struct MeshAttrib {

        // TODO: Move mesh_index here. ?
        // TODO: Move mesh shader here. ?

        // NOTE: This only works with shader programs
        //       which implement wind in their vertex shaders.
        bool affected_by_wind   { false };
        
        bool render_backface    { false };
        Color tint              { WHITE };
        float shine             { 0.3f };
   
    };

    // --- Renderable Load Flags --------

    // Enables ability to change individual mesh transform matrices
    // through 'Renderable::mesh_transform' array.
    static constexpr int RLF_MESH_TRANSFORMS = 1 << 0;
    
    // If this flag is used animations for model
    // are loaded from the same file.
    static constexpr int RLF_ANIMATIONS = 1 << 1;
    
    // ----------------------------------


    static constexpr size_t RENDERABLE_MAX_NAME_SIZE = 24;


    class Renderable {
        public:

            void load(const char* path,
                    std::initializer_list<Shader> shaders,  int load_flags = 0);

            // Guranteed to be NULL terminated.
            char name[RENDERABLE_MAX_NAME_SIZE];

            void unload();
            void render();
            void mesh_attribute(size_t mesh_index, const MeshAttrib& mesh_attrib);

            bool is_loaded() { return m_loaded; }
            Matrix* transform;

            // Size is number of meshes in model. 
            // Only available if ENABLE_MESH_TRANSFORMS is set used.
            Matrix* mesh_transforms { NULL };

            AM::Animation  anim;
            void           update_animation(float frame_time);

            uint32_t num_meshes()  { return (uint32_t)m_model.meshCount; }
            Model* get_model()     { return &m_model; }
        private:

            MeshAttrib* m_mesh_attribs;
            //std::vector<MeshAttrib> m_mesh_attribs;
            bool m_loaded { false };

            Model m_model;

            void m_name_from_path(const char* path);
    };
}



#endif
