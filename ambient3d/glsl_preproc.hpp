#ifndef AMBIENT3D_GLSL_PREPROCESSOR_HPP
#define AMBIENT3D_GLSL_PREPROCESSOR_HPP

#include <string>


/*

    It can include files with `#include "filename.glsl"`
    or code can be found from memory, 
    but the it must first know that it exists
    for example this will allow `#include @USEFUL_STUFF` to be used in shaders:
        `AM::GLSL_preproc_add_meminclude("USEFUL_STUFF", code);`
        "code" refers to the glsl code that is then found with @USEFUL_STUFF in the shader.

*/

namespace AM {

    enum PREPROC_FLAGS : int {
        DEFINE__RENDER_INSTANCED = 1 // Adds "#define RENDER_INSTANCED"
    };

    std::string GLSL_preproc(std::string code, int flags = 0);
    void GLSL_preproc_add_meminclude(const std::string& tag_name, const std::string& code);
};





#endif
