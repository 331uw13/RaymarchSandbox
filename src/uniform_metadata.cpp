#include <cstdio>
#include <cstring>

#include "uniform_metadata.hpp"
#include "internal_lib.hpp"


template<typename F, typename Arguments>
static void UniformMetadata_foreach(F fn, Arguments args) {
    printf("%s\n", __func__);

    const std::string shader_code = std::get<0>(std::make_tuple(args));

    std::string::size_type begin_idx = shader_code.find(UniformMetadata::TAG_BEGIN);
    std::string::size_type   end_idx = shader_code.find(UniformMetadata::TAG_END);

    if((begin_idx == std::string::npos) || (end_idx == std::string::npos)) {
        fprintf(stderr, "%s @ %s: No valid uniform metadata.\n",
                __FILE__, __func__);
        return;
    }

    constexpr size_t tag_begin_len = strlen(UniformMetadata::TAG_BEGIN);
    begin_idx += tag_begin_len;

    std::string lot = shader_code.substr(begin_idx, end_idx - begin_idx);


    constexpr size_t buffer_size = 1024;
    char buffer[buffer_size+1] = { 0 };
    size_t bufidx = 0;

    for(size_t i = 0; i < lot.size(); i++) {
        char c = lot[i];  
        if(c == '\n') {
            fn(buffer);
            memset(buffer, 0, bufidx);
            bufidx = 0;
            continue;
        }
        buffer[bufidx] = c;
        bufidx++;
        if(bufidx >= buffer_size) {
            fprintf(stderr, "%s @ %s: Warning! line is very long, ignored.\n", 
                    __FILE__, __func__);
            memset(buffer, 0, bufidx);
            bufidx = 0;
        }
    }
}


void UniformMetadata::remove(std::string* shader_code) {

    std::string::size_type begin_idx = shader_code->find(UniformMetadata::TAG_BEGIN);
    std::string::size_type   end_idx = shader_code->find(UniformMetadata::TAG_END);

    if((begin_idx == std::string::npos) || (end_idx == std::string::npos)) {
        fprintf(stderr, "%s @ %s: No valid uniform metadata.\n",
                __FILE__, __func__);
        return;
    }

    shader_code->erase(begin_idx, end_idx);
}

void UniformMetadata::write(std::string* shader_code) {
    UniformMetadata::remove(shader_code);


    InternalLib& ilib = InternalLib::get_instance();

    shader_code->append(UniformMetadata::TAG_BEGIN);
    shader_code->push_back('\n');

    for(Uniform u : ilib.uniforms) {

        constexpr size_t buffer_size = 500;
        char buffer[buffer_size+1] = { 0 };
        snprintf(buffer, buffer_size, 
                "    \"%s\"(%s)[%f, %f, %f, %f]\n",
                    u.name.c_str(),
                    UNIFORM_DATA_TYPES_STR[u.type],
                    u.values[0],
                    u.values[1],
                    u.values[2],
                    u.values[3]);

        shader_code->append(buffer);
    };


    shader_code->push_back('\n');
    shader_code->append(UniformMetadata::TAG_END);
    shader_code->push_back('\n');
}

void UniformMetadata::read(const std::string& shader_code) {
   
    UniformMetadata_foreach([](const std::string& line)
    {
        if(line.empty()) {
            return;
        }
        
        //printf("\033[90m%s\033[0m\n", line.c_str());

        // ---- Uniform Name ----

        std::string::size_type name_begin_idx = line.find("\"", 0);
        if(name_begin_idx == std::string::npos) {
            fprintf(stderr, "%s | syntax error: first '\"' expected for finding 'name'.\n",
                    __FILE__);
            return;
        }

        std::string::size_type name_end_idx = line.find("\"", name_begin_idx+1);
        if(name_begin_idx == std::string::npos) {
            fprintf(stderr, "%s | syntax error: second '\"' expected for finding 'name'.\n",
                    __FILE__);
            return;
        }

        name_begin_idx++;
        std::string name_str = line.substr(name_begin_idx, name_end_idx - name_begin_idx);
        //printf("name = '%s'\n", name_str.c_str());


        // ---- Uniform Type ----

        std::string::size_type type_begin_idx = line.find("(", name_end_idx);
        if(type_begin_idx == std::string::npos) {
            fprintf(stderr, "%s | syntax error: '(' expected for finding 'type'.\n",
                    __FILE__);
            return; 
        }
        
        std::string::size_type type_end_idx = line.find(")", type_begin_idx+1);
        if(type_begin_idx == std::string::npos) {
            fprintf(stderr, "%s | syntax error: ')' expected for finding 'type'.\n",
                    __FILE__);
            return; 
        }

        type_begin_idx++;
        std::string type_str = line.substr(type_begin_idx, type_end_idx - type_begin_idx);
        //printf("type = '%s'\n", type_str.c_str());


        UniformDataType datatype = UniformDataType::INVALID;
        for(int i = 0; i < UniformDataType::NUM_TYPES; i++) {
            if(UNIFORM_DATA_TYPES_STR[i] == type_str) {
                datatype = UNIFORM_DATA_TYPES[i];
                break;
            }
        }

        if(datatype == UniformDataType::INVALID) {
            fprintf(stderr, "%s | error: uniform data type \"%s\" is invalid.\n",
                    __FILE__, type_str.c_str());
            return;
        }



        // ---- Uniform Values ----
        
        std::string::size_type values_begin_idx = line.find("[", type_end_idx);
        if(values_begin_idx == std::string::npos) {
            fprintf(stderr, "%s | syntax error: '[' expected for finding 'values'.\n",
                    __FILE__);
            return; 
        }
        
        std::string::size_type values_end_idx = line.find("]", values_begin_idx+1);
        if(values_begin_idx == std::string::npos) {
            fprintf(stderr, "%s | syntax error: ']' expected for finding 'values'.\n",
                    __FILE__);
            return; 
        }

        values_begin_idx++;

        constexpr uint32_t max_num_values = 4;
        float values[max_num_values] = { 0 };
        
        constexpr size_t buffer_size = 32;
        char buffer[buffer_size+1] = { 0 };
        size_t bufidx = 0;
        uint32_t num_values = 0;
       
        for(size_t i = values_begin_idx; i < values_end_idx; i++) {
            char c = line[i];
            bool end = (i+1 >= values_end_idx);
            if((c == ',') || end) { 
                values[num_values] = atof(buffer);
                memset(buffer, 0, bufidx);
                bufidx = 0;
                num_values++;
                if(num_values >= max_num_values) {
                    break;
                }
                continue;
            }

            buffer[bufidx] = c;
            bufidx++;
            if(bufidx >= buffer_size) {
                fprintf(stderr, "%s | error: uniform metadata value is too large.\n",
                        __FILE__);
                return;
            }
        }

        Uniform uniform = (Uniform) {
            .type = datatype,
            .location = -1,
            .values = { values[0], values[1], values[2], values[3] },
            .name = name_str
        };

        InternalLib::get_instance().add_uniform(&uniform);
        

    }, shader_code);
}




