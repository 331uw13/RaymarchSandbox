#ifndef GLSL_PREPROC_HPP
#define GLSL_PREPROC_HPP

#include <string>


namespace Preproc 
{ 

    // outdef is a pointer to the final shader code
    // the function will search for #include tags and add code or definitions to *outdef
    void process_glsl(std::string* shader_code, std::string* outdef);

}




#endif
