#ifndef UNIFORM_METADATA_HPP
#define UNIFORM_METADATA_HPP

#include <string>


struct Uniform;

namespace UniformMetadata
{

    static constexpr auto TAG_BEGIN = "@_UNIFORM_METADATA";
    static constexpr auto TAG_END   = "@_UNIFORM_METADATA_END";


    void remove(std::string* shader_code);
    void write(std::string* shader_code);
    void read(const std::string& shader_code);

};






#endif
