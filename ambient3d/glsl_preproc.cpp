#include <map>
#include <cstring>
#include <cstdio>
#include <filesystem>
#include <fstream>

#include "glsl_preproc.hpp"


namespace {
    static std::map<std::string, std::string> memincludes;
    static constexpr const char* INCLUDE_TAG = "#include";
    static constexpr int INCLUDE_TAG_LENGTH = strlen(INCLUDE_TAG);

    static const std::string 
        RENDER_INSTANCED_STR = "#define RENDER_INSTANCED\n";
};


std::string AM::GLSL_preproc(std::string code, int flags) {


    std::string::size_type search_pos = 0;

    while(true) {
        std::string::size_type include_begin = 
            code.find(::INCLUDE_TAG, search_pos);

        if(include_begin == std::string::npos) {
            break;
        }
        std::string::size_type include_value_begin 
            = include_begin + ::INCLUDE_TAG_LENGTH+1;
        search_pos = include_begin;

        std::string::size_type include_end =
            code.find('\n', include_begin);

        if(include_end == std::string::npos) {
            fprintf(stderr, "ERROR! %s: Could not find where the '%s <value>' ends. No newline was found.\n",
                    __func__, ::INCLUDE_TAG);
            break;
        }

        std::string value = code.substr(include_value_begin, 
                include_end - include_value_begin);

        if(value[0] == '@') {
            // Try to find code to include from hashmap.
            value.erase(0, 1);

            const auto search = ::memincludes.find(value);
            if(search == ::memincludes.end()) {
                fprintf(stderr, "ERROR! %s: Could not find memory include \"%s\"\n",
                        __func__, value.c_str());
                break;
            }

            code.erase(include_begin, include_end - include_begin);
            code.insert(include_begin+1, search->second);
        }
        else
        if(value[0] == '"') {
            // Try to find file to include.

            value.erase(0, 1);
            value.erase(value.size()-1, 1);

            std::ifstream stream(value, std::ios::ate | std::ios::in);
            if(!stream.is_open()) {
                fprintf(stderr, "ERROR! %s: Failed to open file \"%s\"\n",
                        __func__, value.c_str());
            }
           
            size_t file_size = static_cast<size_t>(stream.tellg());
            std::string tmp(file_size, '\0');
            stream.seekg(0);
            stream.read(&tmp[0], file_size);
            
            stream.close();
            code.erase(include_begin, include_end - include_begin);
        
            code.insert(include_begin+1, tmp);
        }
        else {
            fprintf(stderr, "ERROR! %s: Invalid value prefix for \"%s\": '%c'\n",
                    __func__, ::INCLUDE_TAG, value[0]);
            fprintf(stderr, "     '-> %s\n", value.c_str());
            break;
        }
    }

    if(flags == PREPROC_FLAGS::DEFINE__RENDER_INSTANCED) {
        std::string::size_type version_begin = code.find("#version", 0);
        if(version_begin == std::string::npos) {
            fprintf(stderr, "ERROR! %s: The shader seems to not have version."
                            " Cant add definition for instanced rendering.\n",
                            __func__);
            return std::string();
        }
            
        std::string::size_type version_end = code.find('\n', version_begin);
        if(version_end == std::string::npos) {
            fprintf(stderr, "ERROR! %s: Could not find new line after #version while"
                            " trying to add definition for instanced rendering.\n",
                            __func__);
            return std::string();
        }

        code.insert(version_end+1, ::RENDER_INSTANCED_STR);
    } 

    //printf("%s\n", code.c_str());

    return code;
}

    
void AM::GLSL_preproc_add_meminclude(const std::string& tag_name, const std::string& code) {
    ::memincludes.insert(std::make_pair(tag_name, code));
}

