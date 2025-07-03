
#include <string>
#include <cstring>

#include <fstream>

#include "imgui.h"
#include "internal_lib.hpp"



//#define UCOLOR_FUNC (u8col_t){ 70, 80, 150 }
//#define UCOLOR_USRFUNC (u8col_t) { 150, 70, 130 }
//#define UCOLOR_INFO (u8col_t){ 158, 80, 70 }
//#define UCOLOR_STRUCT (u8col_t){ 130, 70, 150 }
//#define UCOLOR_SDF (u8col_t){ 40, 150, 100 }


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
    this->uniforms.clear();

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

    this->lines = 0;

    while(std::getline(file, line)) {
       
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


        this->lines++;
        this->source += line + '\n';
    }

    /*
    this->source += "layout (local_size_x = 1, local_size_y = 1) in;\n";
    this->source += "layout (rgba32f, binding = 8) uniform image2D output_img;\n";
    this->source += "uniform vec2 screen_size;\n";
    this->source += "uniform float time;\n";
    this->source += "uniform float FOV;\n";
    this->source += "uniform float HIT_DISTANCE;\n";
    this->source += "uniform float MAX_RAY_LENGTH;\n";
    this->source += "uniform vec3 CAMERA_INPUT_POS;\n";
    this->source += "uniform float CAMERA_INPUT_YAW;\n";
    this->source += "uniform float CAMERA_INPUT_PITCH;\n";
    this->source += "#define PI 3.14159\n";
    this->source += "#define PI2 (PI*2.0)\n";
    this->source += "#define PI_R (PI/180.0)\n";
    this->source += "#define ColorRGB(r,g,b) vec3(r/255.0, g/255.0, b/255.0)\n";
    this->source += "#define RAINBOW_PALETTE vec3(0.5, 0.5, 0.5), vec3(0.5, 0.5, 0.5),"
                                             "vec3(1.0, 1.0, 1.0), vec3(0.0, 0.33, 0.67)\n";

    // Create some kind of region that can be found easily.
    // this will allow to edit the glsl code to have custom uniforms.
    // they can be added and removed at runtime.
    this->source += CUSTOM_UNIFORMS_TAG_BEGIN;
    this->source += "\n";
    this->source += CUSTOM_UNIFORMS_TAG_END;


    const char* MATERIAL_DEFINITIONS = 
        "#define Material mat4x4\n"
        "#define Mdiffuse(x)  x[0]\n"
        "#define Mspecular(x) x[1]\n"
        "#define Mdistance(x) x[2][0]\n"
        "#define Mshine(x)    x[2][1]\n"
        "#define Mglow(x)     x[2][2]\n"
        ;
    */

    //printf("%s\n", this->source.c_str());




    /*
    add_document(
            "Material MaterialMin(Material a, Material b)\n"
            "{\n"
            "   return (Mdistance(a) < Mdistance(b)) ? a : b;\n"
            "}\n"
            ,
            "Returns material which distance is smaller.\n"
            "Can be used to add Material to a 'map' function result or other things.\n"
            ,
            UCOLOR_FUNC
            );

    add_document(
            "Material MaterialMax(Material a, Material b)\n"
            "{\n"
            "   return (Mdistance(a) > Mdistance(b)) ? a : b;\n"
            "}\n"
            ,
            "Returns material which distance is bigger."
            ,
            UCOLOR_FUNC
            );

    add_document(
            "Material MixMaterial(Material a, Material b, float t)\n"
            "{\n"
            "  Material m = Material(0);\n"
            "  Mdiffuse(m) = mix(Mdiffuse(a), Mdiffuse(b), t);\n"
            "  Mdistance(m) = mix(Mdistance(a), Mdistance(b), t);\n"
            "  Mspecular(m) = mix(Mspecular(a), Mspecular(b), t);\n"
            "  Mdistance(m) = mix(Mdistance(a), Mdistance(b), t);\n"
            "  Mshine(m)    = mix(Mshine(a), Mshine(b), t);\n"
            "  return m;\n"
            "}\n"
            ,
            ""
            ,
            UCOLOR_FUNC
            );

    add_document(
            "Material BlendMaterials(Material a, Material b, float k)\n"
            "{\n"
            "   float d1 = Mdistance(a);\n"
            "   float d2 = Mdistance(b);\n"
            "   float h = clamp(0.5 + 0.5 * (d2 - d1) / k, 0.0, 1.0);\n"
            "   Material m = MixMaterial(b, a, h);\n"
            "   Mdistance(m) -= k * h * (1.0-h);\n"
            "   return m;\n"
            "}\n"
            ,
            "Smooth Mix."
            ,
            UCOLOR_FUNC
            ,
            "https://iquilezles.org/articles/distfunctions/"
            );
    // https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
    add_document(
            "float Noise(vec2 xy)\n"
            "{\n"
            "   float PHI = 1.61803398874989484820459;\n"
            "   return fract(tan(distance(xy*PHI, xy))*xy.y);\n"
            "}\n"
            ,
            "Returns a pseudo random number.\n"
            ,
            UCOLOR_FUNC
            );


    add_document(
            "vec3 RayDir()\n"
            "{\n"
            "   vec2 rs = screen_size*0.5;\n"
            "   float hf = tan((90.0-FOV*0.5)*(PI_R));\n"
            "   return normalize(vec3(gl_FragCoord.xy-rs, (rs.y*hf)));\n"
            "}\n"
            ,
            "Calculates initial ray direction.\n"
            ,
            UCOLOR_FUNC
            );


    add_document(
            "vec3 ComputeNormal(vec3 p)\n"
            "{\n"
            "   vec2 e = vec2(0.01, 0.0);\n"
            "   return normalize(vec3(\n"
            "      Mdistance(map(p - e.xyy)) - Mdistance(map(p + e.xyy)),\n"
            "      Mdistance(map(p - e.yxy)) - Mdistance(map(p + e.yxy)),\n"
            "      Mdistance(map(p - e.yyx)) - Mdistance(map(p + e.yyx))\n"
            "   ));\n"
            "}\n"
            ,
            "This function will output normal for given point 'p'\n"
            "by sampling the same point but slightly different offsets.\n"
            ,
            UCOLOR_FUNC
            );

    // TODO: The light doesnt have radius currently.
    add_document(
            "vec3 ComputeLight(vec3 light_pos, vec3 color, vec3 normal, vec3 ray_pos, Material m)\n"
            "{\n"
            "   vec3 light_dir = -normalize(light_pos - ray_pos);\n"
            "   vec3 view_dir = normalize(Camera.pos - ray_pos);\n"
            "   vec3 halfway_dir = normalize(light_dir - view_dir);\n"
            "   float nh_dot = max(dot(normal, halfway_dir), 0.0);\n"
            "   float shine = 32 - clamp(Mshine(m), 0, 32);\n"
            "   vec3 specular = color + (pow(nh_dot, shine) * Mspecular(m));\n"
            "   float diffuse = max(dot(normal, light_dir), 0.0);\n"
            "   return (specular * diffuse) * Mdiffuse(m);\n"
            "}\n"
            ,
            "Returns color for the pixel.\n"
            "Notes:\n"
            " - Ambient color is _not_ set by this function.\n"
            " - Material must be valid.\n"
            " - Camera.pos should be where 'ray origin' is.\n"
            ,
            UCOLOR_FUNC
            );
    */
    /*
    // https://iquilezles.org/articles/palettes/
    add_document(
            "vec3 Palette(float t, vec3 a, vec3 b, vec3 c, vec3 d)\n"
            "{\n"
            "   return a + b * cos(PI2 * (c * t + d));\n"
            "}\n"
            ,
            "t: Interpolation\n"
            "a: Color\n"
            "b: Color\n"
            "c: Color\n"
            "d: Color\n"
            "Example: \n"
            "'vec3 color = Palette(sin(time)*0.5+0.5, RAINBOW_PALETTE);'\n"
            ,
            UCOLOR_FUNC
            ,
            "https://iquilezles.org/articles/palettes/"
            );

    add_document(
            "vec3 CameraInputRotation(vec3 rd)\n"
            "{\n"
            "   rd.yz *= RotateM2(CAMERA_INPUT_PITCH);\n"
            "   rd.xz *= RotateM2(CAMERA_INPUT_YAW);\n"
            "   return rd;\n"
            "}\n"
            ,
            "Returns new ray direction.\n"
            "Notes:\n"
            " - Camera input has to be enabled from View_Mode\n"
            ,
            UCOLOR_FUNC
            );

    // https://iquilezles.org/articles/fog/
    add_document(
            "vec3 ApplyFog(vec3 current_color, float t, vec3 fog_color)\n"
            "{\n"
            "   float fog = 1.0 - exp(-t * 2.0);\n"
            "   return mix(current_color, fog_color, fog);\n"
            "}\n"
            ,
            "Returns color for the pixel.\n"
            "t: Distance. Ray.length can be used.\n"
            ,
            UCOLOR_FUNC
            ,
            "https://iquilezles.org/articles/fog/"
            );

    add_document(
            "vec2 Hash2(vec2 x)\n"
            "{\n"
            "   return fract(sin(\n"
            "      vec2(\n"
            "         dot(x, vec2(95.28, 17.75)),\n"
            "         dot(x, vec2(76.32, 32.71))\n"
            "   ))*43758.5453);\n"
            "}\n"
            ,
            "Returns a pseudo random 2D vector.\n"
            ,
            UCOLOR_FUNC
            );

    add_document(
            "vec3 Hash3(vec3 x)\n"
            "{\n"
            "   return fract(sin(\n"
            "      vec3(\n"
            "         dot(x, vec3(1.0, 57.0, 113.0)),\n"
            "         dot(x, vec3(57.0, 113.0, 1.0)),\n"
            "         dot(x, vec3(113.0, 1.0, 57.0))\n"
            "   ))*43758.5453);\n"
            "}\n"
            ,
            "Returns a pseudo random 3D vector.\n"
            ,
            UCOLOR_FUNC
            );

    // https://iquilezles.org/articles/smoothvoronoi/
    add_document(
            "float SmoothVoronoi2D(vec2 x, float falloff, float k)\n"
            "{\n"
            "   vec2 p = floor(x);\n"
            "   vec2 f = fract(x);\n"
            "   float res = 0.0;\n"
            "   for(int y = -1; y <= 1; y++) {\n"
            "      for(int x = -1; x <= 1; x++) {\n"
            "         vec2 b = vec2(float(x), float(y));\n"
            "         vec2 r = vec2(b) - f + Hash2(p + b);\n"
            "         float d = dot(r, r);\n"
            "         res += 1.0 / pow(d, k);\n"
            "      }\n"
            "   }\n"
            "   return pow(1.0 / res, 1.0/falloff);\n"
            "}\n"
            ,
            ""
            ,
            UCOLOR_FUNC
            ,
            "https://iquilezles.org/articles/smoothvoronoi/"
            );

    add_document(
            "vec3 SmoothVoronoi3D(vec3 x, float falloff, float k)\n"
            "{\n"
            "   vec3 p = floor(x);\n"
            "   vec3 f = fract(x);\n"
            "   vec3 res = vec3(0.0);\n"
            "   for(int z = -1; z <= 1; z++) {\n"
            "      for(int y = -1; y <= 1; y++) {\n"
            "         for(int x = -1; x <= 1; x++) {\n"
            "            vec3 b = vec3(float(x), float(y), float(z));\n"
            "            vec3 r = vec3(b) - f + Hash3(p + b);\n"
            "            float d = dot(r, r);\n"
            "            res += 1.0 / pow(d, k);\n"
            "         }\n"
            "      }\n"
            "   }\n"
            "   return pow(1.0 / res, vec3(1.0/falloff));\n"
            "}\n"
            ,
            "Expanded into 3D from iq's Smooth voronoise.\n"
            ,
            UCOLOR_FUNC
            ,
            "https://iquilezles.org/articles/smoothvoronoi/"
            );

    */
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


    struct document_t document = (struct document_t) {
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

    struct document_t document = (struct document_t) {
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
        

std::string InternalLib::get_uniform_code_line(struct uniform_t* u) {
    std::string code = "uniform ";
    code += UNIFORM_DATA_TYPES_STR[u->type];
    code += " " + u->name + ";\n";

    return code;
}

void InternalLib::remove_uniform(struct uniform_t* u) {
    std::string code = get_uniform_code_line(u);

    size_t index = this->source.find(code);
    if(index == std::string::npos) {
        fprintf(stderr, "'%s': '%s' Not found in internal lib source.\n",
                __func__, code.c_str());
        return;
    }

    this->source.erase(index, code.size());
}

void InternalLib::add_uniform(struct uniform_t* u) {
    this->uniforms.push_back(*u);

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



