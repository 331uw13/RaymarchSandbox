#ifndef AMBIENT3D_INTERNAL_SHADERS_HPP
#define AMBIENT3D_INTERNAL_SHADERS_HPP




namespace AM {
    namespace I_Shaders {

        static constexpr const char*
            GLSL_VERSION = "#version 430\n";

        static constexpr const char*
            DEFAULT_VERTEX = R"(

            #include @GLSL_VERSION

            in vec3 vertexPosition;
            in vec2 vertexTexcoord;
            in vec3 vertexNormal;
            in vec4 vertexColor;
           
            #ifdef RENDER_INSTANCED
                in mat4 instanceTransform;
            #endif

            uniform mat4 mvp;
            uniform mat4 matModel;
            uniform mat4 matNormal;
           
            uniform int    u_affected_by_wind;
            uniform float  u_time;

            out vec2 frag_texcoord;
            out vec4 frag_color;
            out vec3 frag_normal;
            out vec3 frag_position;

            void main() {
                
                frag_texcoord = vertexTexcoord;
                frag_color = vertexColor;
                vec3 vertex_pos = vertexPosition;

                if(u_affected_by_wind == 1) {
                    float T = u_time * 3.0;
                    vertex_pos.x += sin(T*0.8523 + vertex_pos.y + cos(T*2.152+vertex_pos.y*1.52)*0.3)*0.3;
                    vertex_pos.z += cos(T*0.2583 + vertex_pos.y + sin(T*1.822+vertex_pos.y*1.73)*0.3)*0.3;
                }
                #ifdef RENDER_INSTANCED
                    frag_position = vec3(instanceTransform*vec4(vertex_pos, 1.0)); 
                    frag_normal = vec3(instanceTransform * vec4(vertexNormal, 0.0));
                    gl_Position = mvp*instanceTransform*vec4(vertex_pos, 1.0);
                #else
                    frag_position = vec3(matModel*vec4(vertex_pos, 1.0));
                    frag_normal = normalize(vec3(matNormal * vec4(vertexNormal, 1.0)));
                    gl_Position = mvp * vec4(vertex_pos, 1.0);
                #endif
            }

            )";


        static constexpr const char*
            DEFAULT_FRAGMENT = R"(
            #include @GLSL_VERSION
            #include @AMBIENT3D_LIGHTS
            
            in vec2 frag_texcoord;
            in vec4 frag_color;
            in vec3 frag_normal;
            in vec3 frag_position;


            uniform sampler2D texture0;
            uniform vec4 colDiffuse;
            uniform vec3 u_view_pos;
            uniform float u_material_shine;

            out vec4 out_color;

            void main() {
                vec4 tex = texture(texture0, frag_texcoord);
                if(tex.a < 0.5) { discard; }
                vec3 lights = compute_lights(
                    frag_position,
                    frag_normal,
                    u_view_pos,
                    u_material_shine
                    );
                out_color = (tex * colDiffuse) * vec4(lights, 1.0);
            }
            )";

        static constexpr const char*
            LIGHTS_GLSL = R"(
            #define MAX_LIGHTS 64

            struct Light {
                vec4 pos;
                vec4 color;
                vec4 settings;
            };

            layout(std140, binding = 1) uniform lights_ubo {
                Light lights[MAX_LIGHTS];
                int num_lights;
            };

            // Returns: RGB.
            vec3 compute_lights(
                vec3 frag_pos,
                vec3 frag_n,
                vec3 view_pos,
                float material_shine
            ){
                vec3 final = vec3(0, 0, 0);
                vec3 normal = normalize(frag_n);
                //normal = mix(normal, vec3(0.0, 1.0, 0.0), 0.3);
                for(int i = 0; i < num_lights; i++) {
                    vec3 light_pos = lights[i].pos.xyz;
                    vec3 light_dir = normalize(light_pos - frag_pos);
                    vec3 view_dir = normalize(view_pos - frag_pos);
                    vec3 halfway_dir = normalize(light_dir - view_dir);

                    vec3 light_color = lights[i].color.rgb;
                    float radius = lights[i].settings.x;
                    float cutoff = lights[i].settings.y;


                    // Diffuse.
                    float diff = max(dot(normal, light_dir), 0.0);
                    vec3 diffuse = diff * light_color;

                    // Specular.
                    float spec = pow(max(dot(view_dir, reflect(-light_dir, normal)), 0.0), material_shine);
                    vec3 specular = spec * mix(vec3(1.0, 1.0, 1.0), light_color, 0.675);
                    specular *= material_shine;

                    // Attenuation.
                    float L = 0.8;
                    float Q = 2.3;
                    float dist = distance(frag_pos, light_pos) / radius;
                    dist = pow(dist, cutoff);
                    float a = 1.0 / (2.0 + L * dist + Q * (dist * dist));
                    
                    diffuse *= a;
                    specular *= (a*0.5);
                    vec3 ambient = a * ((light_color * dist) * 0.15);
                    specular = clamp(specular, vec3(0), vec3(1));
                    final += diffuse + specular + ambient;
                }

                return clamp(final, vec3(0), vec3(1));
            }

            )";

        static constexpr const char*
            POSTPROCESS_FRAGMENT = R"(
            #include @GLSL_VERSION
            
            in vec2 frag_texcoord;
            in vec4 frag_color;
            in vec3 frag_normal;
            in vec3 frag_position;


            uniform float u_vision_effect;
            uniform float u_time;
            uniform sampler2D texture_result;
            layout (location = 3) uniform sampler2D texture_bloom;
           
            out vec4 out_color;

            vec3 vision_effect() {
                float k = u_vision_effect * 3.0;

                vec2 texture_size = vec2(textureSize(texture_result, 0));
                vec2 uv = gl_FragCoord.xy / texture_size;
                vec2 tx = 1.0 / texture_size;
                
                tx.x += cos(u_time)*tx.x;
                
                float tM = u_vision_effect * 3.0 + 0.5*floor(cos(u_time*0.5+(0.5+0.5*sin(u_time*0.05)))*10.0);

                float rX = cos(tM*u_time+uv.y*30) * (tx.x * k);
                float rY = sin(tM*u_time+uv.x*30) * (tx.y * k);
                
                float gX = cos(tM*u_time+uv.x*30) * (tx.x * k);
                float gY = sin(tM*u_time+uv.y*30) * (tx.y * -k);
                
                float bX = cos(tM*u_time+uv.y*30) * (tx.x * -k);
                float bY = sin(tM*u_time+uv.y*30) * (tx.y * k);

                float red    = texture(texture_result, frag_texcoord+vec2(rX, rY)).r;
                float green  = texture(texture_result, frag_texcoord+vec2(gX, gY)).g;
                float blue   = texture(texture_result, frag_texcoord+vec2(bX, bY)).b;
            
                float noise = fract(cos(dot(vec2(-uv.x*20, fract(uv.y)+uv.y*20), vec2(52.621,67.1262)))*72823.53)/40.0;
                float uvl = clamp(pow(length(uv-0.5), 2.0)*3.0+noise, 0.0, 1.0);

                return vec3(red, green+blue*0.5, blue) * uvl;
            }

            void main() {
                
                vec4 result = texture(texture_result, frag_texcoord);
                vec3 bloom = texture(texture_bloom, frag_texcoord).rgb;
                
                result.rgb += bloom;
               
                if(u_vision_effect > 0) {
                    result.rgb += vision_effect();
                }
                if(u_vision_effect > 0.5) {
                    float modulation = sin(u_time * u_vision_effect)*0.5+0.5;
                    float pulse = 1.0-pow((modulation * (sin(u_time * 5.0))*0.5+0.5), 5.0);
                    result.rgb *= pow(pulse, 0.5);
                }

                out_color = result;
                out_color = vec4(pow(out_color.rgb, vec3(1.0/1.15)), out_color.a);
            }
            )";

        static constexpr const char*
            BLOOM_TRESH_FRAGMENT = R"(
            #include @GLSL_VERSION
            
            in vec2 frag_texcoord;
            in vec4 frag_color;
            in vec3 frag_normal;
            in vec3 frag_position;

            uniform sampler2D texture_result;
            uniform sampler2D bloom_result;
           
            out vec4 out_color;

            void main() {
                vec4 black = vec4(0.0, 0.0, 0.0, 1.0);
                vec4 result = texture(texture_result, frag_texcoord);
                float light = dot(result.rgb, vec3(0.5, 1.0, 1.0));
                
                out_color.rgb = mix(black, result, pow(light, 4.0)).rgb*0.5;
                out_color.a = result.a;    
            }
            )";

        static constexpr const char*
            BLOOM_DOWNSAMPLE_FRAGMENT = R"(
            #include @GLSL_VERSION
            
            in vec2 frag_texcoord;
            in vec4 frag_color;
            in vec3 frag_normal;
            in vec3 frag_position;

            uniform sampler2D texture0;

            out vec4 out_color;

            void main() {
               
                int size = 2;
                vec2 texture_size = vec2(textureSize(texture0, 0));
                vec2 texel_size = 1.0 / texture_size;
                vec2 uv = gl_FragCoord.xy / texture_size;

                vec3 result = vec3(0, 0, 0);
                float sum = 0.0;

                for(int x = -size; x <= size; x++) {
                    for(int y = -size; y <= size; y++) {
                        vec2 offset = vec2(float(x), float(y));
                        vec2 texel_pos = frag_texcoord + offset * texel_size;
                        if(texel_pos.y > 0.99) { break; }
                        if(texel_pos.y < 0.01) { break; }
                        if(texel_pos.x > 0.99) { break; }
                        if(texel_pos.x < 0.01) { break; }

                        sum += 1.0;
                        result += texture(texture0, texel_pos).rgb;
                    }
                }

                result /= sum;
                out_color = vec4(result, 1.0);
            }
            )";

        static constexpr const char*
            BLOOM_UPSAMPLE_FRAGMENT = R"(
            #include @GLSL_VERSION
            
            in vec2 frag_texcoord;
            in vec4 frag_color;
            in vec3 frag_normal;
            in vec3 frag_position;

            uniform sampler2D texture0;
           
            out vec4 out_color;

            void main() {
                
                int size = 2;
                vec2 texture_size = vec2(textureSize(texture0, 0));
                
                vec2 texel_size = 1.0 / texture_size;
                vec2 uv = gl_FragCoord.xy / texture_size;
                vec3 result = vec3(0, 0, 0);
                float sum = 0;

                for(int x = -size; x <= size; x++) {
                    for(int y = -size; y <= size; y++) {
                        vec2 offset = vec2(float(x), float(y));
                        vec2 texel_pos = frag_texcoord + offset * texel_size;
                        
                        if(texel_pos.y > 0.99) { break; }
                        if(texel_pos.y < 0.01) { break; }
                        if(texel_pos.x > 0.99) { break; }
                        if(texel_pos.x < 0.01) { break; }

                        float dist = distance(frag_texcoord, texel_pos);

                        result += texture(texture0, texel_pos).rgb * dist;
                        sum += dist;
                    }
                }

                result /= sum;
                out_color = vec4(result, 1.0);
            }
            )";


    };
};






#endif
