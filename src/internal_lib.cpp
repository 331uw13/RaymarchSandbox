
#include <string>
#include <cstring>

#include "imgui.h"
#include "internal_lib.hpp"

#define GLSL_VERSION "#version 430\n"

void InternalLib::create_source() {
    
    this->source = "";
    this->documents.clear();
    this->uniforms.clear();


    this->source += GLSL_VERSION;
    this->source += "out vec4 out_color;\n";
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
        "#define Material mat3x3\n"
        "#define Mdiffuse(x)  x[0]\n"
        "#define Mspecular(x) x[1]\n"
        "#define Mdistance(x) x[2][0]\n"
        "#define Mshine(x)    x[2][1]\n"
        "#define Mglow(x)     x[2][2]\n"
        ;

    this->source += MATERIAL_DEFINITIONS;


    add_document(
            "struct CAMERA_T\n"
            "{\n"
            "   vec3 pos;\n"
            "   vec3 dir;\n"
            "} Camera;\n"
            ,
            "Needed for light calculations\n"
            "Camera position should be the view position\n"
            "(usually the same as ray origin).\n"
            "Camera direction is not yet used.\n"
            );

    add_document(
            "struct RAY_T\n"
            "{\n"
            "   int hit;\n"
            "   vec3 pos;\n"
            "   float length;\n"
            "   Material material;\n"
            "   Material closest;\n"
            "} Ray;\n"
            ,
            "'Raymarch(..)' functions set these global variables.\n"
            );

    add_document(
            "Material map(vec3 p);"
            ,
            "User must define this function.\n"
            );

    add_document(
            "Material MaterialMin(Material a, Material b)\n"
            "{\n"
            "   return (Mdistance(a) < Mdistance(b)) ? a : b;\n"
            "}\n"
            ,
            "Returns material which distance is smaller.\n"
            "Can be used to add Material to a 'map' function result or other things.\n"
            );

    add_document(
            "Material MaterialMax(Material a, Material b)\n"
            "{\n"
            "   return (Mdistance(a) > Mdistance(b)) ? a : b;\n"
            "}\n"
            ,
            "Returns material which distance is bigger."
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
            "https://iquilezles.org/articles/distfunctions/"
            );

    // https://iquilezles.org/articles/distfunctions/
    add_document(
            "float SphereSDF(vec3 p, float radius)\n"
            "{\n"
            "    return length(p)-radius;\n"
            "}\n"
            ,
            "Signed distance to sphere.\n"
            ,
            "https://iquilezles.org/articles/distfunctions/"
            );
    add_document(
            "float BoxSDF(vec3 p, vec3 size)\n"
            "{\n"
            "   vec3 q = abs(p) - size;\n"
            "   return length(max(q, 0.0)) + min(max(q.x, max(q.y,q .z)), 0.0);\n"
            "}\n"
            ,
            "Signed distance to box.\n"
            ,
            "https://iquilezles.org/articles/distfunctions/"
            );

    add_document(
            "float BoxFrameSDF(vec3 p, vec3 size, float e)\n"
            "{\n"
            "  p = abs(p) - size;\n"
            "  vec3 q = abs(p + e) - e;\n"
            "  return min(min(\n"
            "   length(max(vec3(p.x,q.y,q.z),0.0))+min(max(p.x,max(q.y,q.z)),0.0),\n"
            "   length(max(vec3(q.x,p.y,q.z),0.0))+min(max(q.x,max(p.y,q.z)),0.0)),\n"
            "   length(max(vec3(q.x,q.y,p.z),0.0))+min(max(q.x,max(q.y,p.z)),0.0));\n"
            "}\n"
            ,
            "Signed distance to box frame.\n"
            ,
            "https://iquilezles.org/articles/distfunctions/"
            );

    add_document(
            "float TorusSDF(vec3 p, vec2 t)\n"
            "{\n"
            "   vec2 q = vec2(length(p.xz) - t.x, p.y);\n"
            "   return length(q) - t.y;\n"
            "}\n"
            ,
            "Signed distance to torus.\n"
            ,
            "https://iquilezles.org/articles/distfunctions/"
            );

    // https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
    add_document(
            "float Noise(vec2 xy, float seed)\n"
            "{\n"
            "   float PHI = 1.61803398874989484820459;\n"
            "   return fract(tan(distance(xy*PHI, xy) * seed)*xy.y);\n"
            "}\n"
            ,
            "Returns a pseudo random number.\n"
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
            );

    add_document(
            "void Raymarch(vec3 ro, vec3 rd)\n"
            "{\n"
            "   Ray.material = Material(0);\n"
            "   Ray.closest = Material(0);\n"
            "   Mdistance(Ray.closest) = MAX_RAY_LENGTH+1;\n"
            "\n"
            "   Ray.hit = 0;\n"
            "   Ray.length = 0.0;\n"
            "   Ray.pos = ro;\n"
            "\n"
            "   for(; Ray.length < MAX_RAY_LENGTH; ) {\n"
            "      Ray.pos = ro + rd * Ray.length;\n"
            "      Material closest = map(Ray.pos);\n"
            "      if(Mglow(closest) > 0.0 && Mdistance(closest) < Mdistance(Ray.closest)) {\n"
            "         Ray.closest = closest;\n"
            "      }\n"
            "      if(Mdistance(closest) <= HIT_DISTANCE) {\n"
            "         Ray.hit = 1;\n"
            "         Ray.material = closest;\n"
            "         break;\n"
            "      }\n"
            "      Ray.length += Mdistance(closest);\n"
            "   }\n"
            "}\n"
            ,
            "ro: Ray origin\n"
            "rd: Ray direction\n"
            "Results of this function can be\n"
            "accessed from 'Ray' struct.\n"
            "Notes:\n"
            " - rd must be normalized\n"
            " - user must define 'map' function\n"
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
            );


    // https://iquilezles.org/articles/sdfrepetition/
    add_document(
            "vec3 RepeatINF(vec3 p, vec3 s)\n"
            "{\n"
            "   return p - s * round(p / s);\n"
            "}\n"
            ,
            "Repeat space \"infinite\".\n"
            "p: Current ray position.\n"
            "s: Grid size.\n"
            ,
            "https://iquilezles.org/articles/sdfrepetition/"
            );
    add_document(
            "vec3 RepeatLIM(vec3 p, vec3 s, vec3 lim)\n"
            "{\n"
            "   return p - s * clamp(round(p / s), -lim, lim);\n"
            "}\n"
            ,
            "Repeat space limited times.\n"
            "p: Current ray position.\n"
            "s: Grid size\n"
            "lim: Limit\n"
            ,
            "https://iquilezles.org/articles/sdfrepetition/"
            );


    add_document(
            "mat3 RotateM3(vec2 angle)\n"
            "{\n"
            "   vec2 c = cos(angle);\n"
            "   vec2 s = sin(angle);\n"
            "   return mat3(\n"
            "      c.y,      0.0,  -s.y,\n"
            "      s.y*s.x,  c.x,   c.y*s.x,\n"
            "      s.y*c.x, -s.x,   c.y*c.x );\n"
            "}\n"
            ,
            "Returns 3x3 rotation matrix\n"
            "Example: 'p.xyz *= RotateM3(vec2(time, time*0.25));'\n"
            );

    add_document(
            "mat2 RotateM2(float angle)\n"
            "{\n"
            "   float c = cos(angle);\n"
            "   float s = sin(angle);\n"
            "   return mat2(c, -s, s, c);\n"
            "}\n"
            ,
            "Returns 2x2 rotation matrix\n"
            );


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
            "https://iquilezles.org/articles/smoothvoronoi/"
            );

}


void InternalLib::add_document(const char* code, const char* description, const char* link) {
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
        .num_newlines = 0
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




