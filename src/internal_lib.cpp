
#include <string>
#include <cstring>

#include <fstream>

#include "imgui.h"
#include "internal_lib.hpp"


static const struct u8col_t UCOLOR_INFO    = (u8col_t){ 130, 50, 60 };
static const struct u8col_t UCOLOR_STRUCT  = (u8col_t){ 100, 70, 120 };
static const struct u8col_t UCOLOR_USRFUNC = (u8col_t){ 100, 50, 80 };
static const struct u8col_t UCOLOR_SDF     = (u8col_t){ 40, 100, 60 };
static const struct u8col_t UCOLOR_FUNC    = (u8col_t){ 50, 40, 100 };


static bool first_token_match(const std::string& line, const char* str, size_t* first_size_out) {
    size_t i = line.find(" ", 0);
    if(first_size_out) {
        *first_size_out = i+1;
    }
    if(i == std::string::npos) {
        return false;
    }

    return (line.substr(0, i) == str);
}

static bool str_contains(const std::string& str, size_t pos, const char* part, size_t part_size) {
    const size_t str_size = str.size();
    if(str_size == 0) {
        return false;
    }
    if(pos >= str_size) {
        pos = str_size-1;
    }

    bool isfound = false;
    size_t k = 0;

    for(size_t i = pos; i < str.size(); i++) {
        if(str[i] != part[k]) {
            k = 0;
            continue;
        }

        k++;
        if(k >= part_size) {
            isfound = true;
            break;
        }
    }

    return isfound;
}

void InternalLib::create_source() {
    this->source = "";
    this->documents.clear();
    this->clear_uniforms();

    add_info("About 'Material'"
            ,
            "Material can be initialized with 'Material m = EmptyMaterial()'\n"
            "Following properties can be modified.\n"
            "(vec3)  |  Mdiffuse(m)  = <Diffuse color>\n"
            "(vec3)  |  Mspecular(m) = <Specular color>\n"
            "(float) |  Mdistance(m) = <SDF Function result>\n"
            "(float) |  MreflectN(m) = <0 = non-reflective(default),  1 = reflective>\n"
            "(float) |  Mopaque(m)   = <0.0 = fully opaque(default),  0.0 - 1.0 = transparent>\n"
            "(float) |  Mcanglow(m)  = when set to 1.0 or above the material will contribute to Ray.closest_mat\n"
            ,
            UCOLOR_INFO);


    // Create some kind of region that can be found easily.
    // this will allow to edit the glsl code to have custom uniforms.
    // they can be added and removed at runtime.
    this->source += CUSTOM_UNIFORMS_TAG_BEGIN;
    this->source += "\n";
    this->source += CUSTOM_UNIFORMS_TAG_END;


    // The reason why functions, structures and information about them
    // is parsed in this kind of way is because it is convenient to have the documents 
    // inside the tool itself.

    std::ifstream file("internal.glsl");
    std::string line;

    bool read_code = false;
    bool read_func_info = false;
    std::string buf = "";
    std::string info_buf = "";
    struct u8col_t document_color = UCOLOR_INFO;

    this->num_lines = 0;

    while(std::getline(file, line)) {
        if(line.empty()) { continue; } 
        this->num_lines++;
       
        if(line == "/* -INFO") { /* Start reading function info */
            read_func_info = true;
            info_buf.clear();
            continue;
        }

        if(read_func_info) { /* Reading function info */
            if(line == "*/") {
                read_func_info = false;
                continue;
            }
            info_buf += line + '\n';
            continue;
        }

        size_t first_token_size = 0;
        if(first_token_match(line, "FUNC", &first_token_size)) {
            line.erase(0, first_token_size);
            buf.clear();
            read_code = true;


            // Choose color for the document.
            if(str_contains(line, first_token_size, "SDF", 3)) {
                document_color = UCOLOR_SDF;
            }
            else
            if(line[line.size()-1] == ';') {
                document_color = UCOLOR_USRFUNC;
            }
            else {
                document_color = UCOLOR_FUNC;
            }
        }
        else
        if(first_token_match(line, "struct", &first_token_size)) {
            buf.clear();
            read_code = true;
            document_color = UCOLOR_STRUCT;
        }
        else
        if((line == "FUNC_END")
        || (line == "};") /* <- Structure/ubo/ssbo scope ended. */
        ){
            // All the info for document is now collected.
            if(line != "FUNC_END") {
                buf += line+'\n';
            }
            read_code = false;
            add_document(buf.c_str(), info_buf.c_str(), document_color);
            continue;
        }

        if(read_code) {
            buf += line + '\n';
            continue;
        }

        this->source += line + '\n';
    }

    // This will reset the line number count.
    // it is being used to track the real line number for error log.
    this->source += "#line 0\n";
}


void InternalLib::add_document(const char* code, const char* description, struct u8col_t color, const char* link) {
    if(!code) {
        fprintf(stderr, "'%s': code must not be empty.\n",
                __func__);
        return;
    }
    if(!description) {
        fprintf(stderr, 
                "'%s': code must have at least"
                " some kind of description.\n",
                __func__);
        return;
    }


    Document document = (Document) {
        .code = code,
        .desc = description,
        .name = "",
        .link = !link ? "" : link,
        .num_newlines = 0,
        .color = color
    };

    // The first line of 'code'. is the function name.
    // Also count the newlines, 
    //   it will be used for rendering text correct size later.
    bool read_name = true;
    size_t code_len = strlen(code);
    for(size_t i = 0; i < code_len; i++) {
        if(code[i] == '\n') {
            read_name = false;
            document.num_newlines++;
        }

        if(read_name) {
            document.name += code[i];
        }
    }

    this->source += code;
    this->documents.push_back(document);
}
        
void InternalLib::add_info(const char* title, const char* description, struct u8col_t color, const char* link) {
    if(!title) {
        fprintf(stderr, "'%s': Info must have a title.\n",
                __func__);
        return;
    }
    if(!description) {
        fprintf(stderr, "'%s': Info must have at least"
                        " some kind of description.\n",
                        __func__);
        return;
    }

    Document document = (Document) {
        .code = "<No source has been set>",
        .desc = description,
        .name = title,
        .link = !link ? "" : link,
        .num_newlines = 0,
        .color = color
    };

    this->documents.push_back(document);
}

const std::string InternalLib::get_source() {
    return this->source;
}
        

std::string InternalLib::get_uniform_code_line(Uniform* u) {
    
    std::string code = "uniform ";
    code += UNIFORM_GLSL_TYPES_STR[u->type];
    code += " " + u->name;

    if(code.back() == '\0') {
        code.replace(code.size()-1, 1, ";");
    }
    else {
        code.push_back(';');
    }

    printf("%s\n", code.c_str());

    return code;
}

void InternalLib::remove_uniform(Uniform* u) {
    std::string code = get_uniform_code_line(u);

    size_t index = this->source.find(code);
    if(index == std::string::npos) {
        fprintf(stderr, "'%s': '%s' Not found in internal lib source.\n",
                __func__, code.c_str());
        return;
    }

    this->source.erase(index, code.size());
}

void InternalLib::add_uniform(Uniform* u) {
    this->uniforms.push_back(*u);

    if(u->type == UniformDataType::TEXTURE) {
        return; // Textures are set to array in 'internal.glsl'
    }

    std::string linebuf = "";
    int64_t found_index = -1;

    for(size_t i = 0; i < this->source.size(); i++) {

        linebuf += this->source[i];
        if(strcmp(linebuf.c_str(), CUSTOM_UNIFORMS_TAG_BEGIN) == 0) {
            found_index = i + 1;
            break;
        }
      
        if(this->source[i] == '\n') {
            linebuf.clear();
            continue;
        }
    }

    if(found_index < 0) {
        fprintf(stderr, "'%s': Failed to find \"%s\" from internal lib source.\n",
                __func__, CUSTOM_UNIFORMS_TAG_BEGIN);
        return;
    }

    this->source.insert(found_index, get_uniform_code_line(u));
}


void InternalLib::clear() {
    this->source.clear();
    this->documents.clear();
}


void InternalLib::clear_uniforms() {
    for(Uniform u : this->uniforms) {
        if(u.has_texture) {
            UnloadTexture(u.texture);
        }
    }

    this->uniforms.clear();
}

