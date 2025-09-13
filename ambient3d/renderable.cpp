#include <cstdio>
#include "renderable.hpp"
#include "ambient3d.hpp"

#include "rlgl.h"

#include "external/glad.h"


void AM::Renderable::load(
        const char* path,
        std::initializer_list<Shader> shaders, int load_flags
){
    if(!FileExists(path)) {
        fprintf(stderr, "ERROR! %s: \"%s\" doesnt exist or no permission to read.\n",
                __func__, path);
        return;
    }

    m_model = LoadModel(path);

    /*
    if(!IsModelValid(m_model)) {
        fprintf(stderr, "ERROR! %s: Failed to load \"%s\"\n",
                __func__, path);
        return;
    }
    */

    if(m_model.meshCount <= 0) {
        fprintf(stderr, "ERROR! %s: (%s) Model doesnt have any meshes.\n",
                __func__, path);
        return;
    }

    if((load_flags & RLF_MESH_TRANSFORMS)) {
        this->mesh_transforms = new Matrix[m_model.meshCount];
    }

    if((load_flags & RLF_ANIMATIONS)) {
        this->anim.load(path);
    }

    m_mesh_attribs = new MeshAttrib[m_model.meshCount];

    size_t shader_idx = 0;
    for(int i = 0; i < m_model.meshCount; i++) {
        m_model.materials[0].shader = *(shaders.begin()+shader_idx);
        m_model.materials[m_model.meshMaterial[i]].shader = *(shaders.begin()+shader_idx);
        if(shader_idx+1 < shaders.size()) {
            shader_idx++;
        }

        if(this->mesh_transforms) {
            this->mesh_transforms[i] = MatrixIdentity();
        }
    }

    // Name will be used for better error messages.
    m_name_from_path(path);

    //mesh_attribute(0, MeshAttrib{}); // Add default mesh attribute.
    this->transform = &m_model.transform;
    m_loaded = true;
}

void AM::Renderable::m_name_from_path(const char* path) {

    const size_t path_size = strlen(path);
    size_t index = path_size;
    for(; index > 0; index--) {
        char c = path[index];
        if(c == '/') { break; }
        if(c == '\\') { break; }
    }
    index++;
    size_t name_len = path_size - index;
    bool name_too_long = (name_len >= RENDERABLE_MAX_NAME_SIZE);
    
    if(name_too_long) {
        name_len = RENDERABLE_MAX_NAME_SIZE-1;
    }

    memmove(this->name, &path[index], name_len);

    if(name_too_long) { 
        memset(&this->name[RENDERABLE_MAX_NAME_SIZE-4], '.', 3);
    }
    this->name[name_len] = '\0';
}

void AM::Renderable::unload() {
    if(!m_loaded) { return; }
    
    if(this->mesh_transforms) {
        delete[] this->mesh_transforms;
        this->mesh_transforms = NULL;
    }

    if(m_mesh_attribs) {
        delete[] m_mesh_attribs;
        m_mesh_attribs = NULL;
    }

    UnloadModel(m_model);
    m_loaded = false;
}
            
void AM::Renderable::mesh_attribute(size_t mesh_index, const MeshAttrib& mesh_attrib) {
    if(mesh_index >= (size_t)m_model.meshCount) {
        fprintf(stderr, "ERROR! %s: (%s) Mesh index out of bounds: %li\n",
                __func__, this->name, mesh_index);
        return;
    }
        
    m_mesh_attribs[mesh_index] = mesh_attrib;
}

void AM::Renderable::render() {
    if(!m_loaded) { return; }

    for(int i = 0; i < m_model.meshCount; i++) {
        Material& mat = m_model.materials[m_model.meshMaterial[i]];

        const MeshAttrib& mesh_attr = (i < m_model.meshCount)
            ? m_mesh_attribs[i] : MeshAttrib{};

        if(mesh_attr.render_backface) {
            rlDisableBackfaceCulling();
        }
        
        mat.maps[MATERIAL_MAP_DIFFUSE].color = mesh_attr.tint;

        AM::set_uniform_int(mat.shader.id, "u_affected_by_wind", mesh_attr.affected_by_wind);
        AM::set_uniform_float(mat.shader.id, "u_material_shine", mesh_attr.shine);
        
        DrawMesh(m_model.meshes[i], mat, 
                (this->mesh_transforms == NULL) 
                ? m_model.transform
                : MatrixMultiply(this->mesh_transforms[i], m_model.transform)
                );
    
        if(mesh_attr.render_backface) {
            rlEnableBackfaceCulling();
        }
        if(mesh_attr.affected_by_wind) {
            AM::set_uniform_int(mat.shader.id, "u_affected_by_wind", 0);
        }
    }
}
            
/*
void AM::Renderable::render_instanced(Matrix* transforms, size_t size) {
    if(!m_loaded) { return; }

    for(int i = 0; i < m_model.meshCount; i++) {
        Material& mat = m_model.materials[m_model.meshMaterial[i]];

        const MeshAttrib& mesh_attr = ((size_t)i < m_model.meshCount)
            ? m_mesh_attribs[i] : MeshAttrib{};

        if(mesh_attr.render_backface) {
            rlDisableBackfaceCulling();
        }

        AM::set_uniform_int(mat.shader.id, "u_affected_by_wind", mesh_attr.affected_by_wind);
        DrawMeshInstanced(m_model.meshes[i], mat, transforms, size);
        
        if(mesh_attr.render_backface) {
            rlEnableBackfaceCulling();
        }
        if(mesh_attr.affected_by_wind) {
            AM::set_uniform_int(mat.shader.id, "u_affected_by_wind", 0);
        }
    }  
}
*/

