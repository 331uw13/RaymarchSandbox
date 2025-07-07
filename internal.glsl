

// https://github.com/331uw13/RaymarchSandbox


layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout (rgba16f, binding = 8) uniform image2D output_img;

uniform vec2 monitor_size;
uniform float time;
uniform float FOV;
uniform float HIT_DISTANCE;
uniform float MAX_RAY_LENGTH;
uniform float CAMERA_INPUT_YAW;
uniform float CAMERA_INPUT_PITCH;
uniform vec3 CameraInputPosition;

#define PI 3.14159
#define PI2 (PI*2.0)
#define PI_R (PI/180.0)
#define ColorRGB(r,g,b) vec3(r/255.0, g/255.0, b/255.0)

#define RAINBOW_PALETTE vec3(0.5, 0.5, 0.5), vec3(0.5, 0.5, 0.5),vec3(1.0, 1.0, 1.0), vec3(0.0, 0.33, 0.67)



#define Material mat4x3
#define Mdiffuse(x)     x[0]
#define Mspecular(x)    x[1]
#define Mdistance(x)    x[2][0]
#define MreflectN(x)    x[2][1]
#define Mopaque(x)      x[2][2]
#define Mcanglow(x)     x[3][0]

/* -INFO
User must define this function.
   - p is the current ray position.
*/
FUNC Material map(vec3 p);
FUNC_END




/* -INFO
TODO: Add more info

User must define this function for colorize transparent materials.
Notes/Tips:
   - Ray.mat      : what material is being processed.
   - Ray.vm_len   : distance enter point to exit point.

*/
FUNC void vm_map();
FUNC_END




/* -INFO
Raymarch function will set these variables.
RAY_T Ray;
*/
struct RAY_T
{
    vec3  volume_color;

    int   hit;
    vec3  pos;
    float len;    // Ray length to first ray hit (reflection doesnt use this.)
    float vm_len; // Distance from enter point to exit point.

    Material mat; // Material which ray hit.
    Material closest_mat; // Closest material to ray.

    float reflect_len; // Ray length after hit to reflective material.
    Material reflectoff_mat;
};
RAY_T Ray;


/* -INFO
Material should be initialized correctly.
*/
FUNC Material EmptyMaterial()
{
    return Material(
            vec3(0.5, 0.5, 0.5), // Default diffuse color.
            vec3(0.2, 0.2, 0.2), // Default specular color.
            vec3(
                MAX_RAY_LENGTH+1.0,
                0, // Non-reflective by default.
                1  // Opaque by default.
                ),
            vec3(
                0,
                0,
                0
                ));
}
FUNC_END

/* -INFO
Set color for the current pixel.
*/
FUNC void SetPixel(vec3 color)
{
    color += Ray.volume_color;
    imageStore(output_img, ivec2(gl_GlobalInvocationID.xy), vec4(color, 1.0));
}
FUNC_END

/* -INFO
Returns 2x2 Rotation matrix.
*/
FUNC mat2 RotateM2(float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return mat2(c, -s, s, c);
}
FUNC_END


/* -INFO
Returns 3x3 Rotation matrix.
*/
FUNC mat3 RotateM3(vec2 angle)
{
    vec2 c = cos(angle);
    vec2 s = sin(angle);
    return mat3(
            c.y,      0.0,  -s.y,
            s.y*s.x,  c.x,   c.y*s.x,
            s.y*c.x, -s.x,   c.y*c.x );
}
FUNC_END

/* -INFO
Returns new ray direction.
   - Camera input has to be enabled from View_Mode (see keybinds tab)
*/
FUNC vec3 CameraInputRotation(vec3 rd)
{
    rd.yz *= RotateM2(CAMERA_INPUT_PITCH);
    rd.xz *= RotateM2(CAMERA_INPUT_YAW);
    return rd;
}
FUNC_END

/* -INFO
Calculate the initial ray direction.
*/
FUNC vec3 Raydir()
{
    vec2 res = imageSize(output_img);
    vec2 id = vec2(gl_GlobalInvocationID.xy);

    float hf = tan((90.0-FOV*0.5)*PI_R);
    return normalize(vec3(id-res*0.5, (res.y*0.5)*hf));
}
FUNC_END


/* -INFO
This function will return surface normal for given point 'p'
by sampling the same point buf slightly different offsets.
*/
FUNC vec3 ComputeNormal(vec3 p)
{
    vec2 e = vec2(0.01, 0.0);
    return normalize(vec3(
        Mdistance(map(p - e.xyy)) - Mdistance(map(p + e.xyy)),
        Mdistance(map(p - e.yxy)) - Mdistance(map(p + e.yxy)),
        Mdistance(map(p - e.yyx)) - Mdistance(map(p + e.yyx))
    ));
}
FUNC_END


int _FLAG_reflect = 0;

/* -INFO
*/
FUNC void Raymarch_I(vec3 ro, vec3 rd)
{    
    Ray.hit = 0;
    Ray.len = 0.0;
    Ray.vm_len = 0.0;
    Ray.pos = ro;
    Ray.mat = Material(0);
    
    int ray_outside = 1;

    while(Ray.len < MAX_RAY_LENGTH) {
        if(ray_outside == 1) {
            Ray.pos = ro + rd * Ray.len;
            Material c = map(Ray.pos);
            if((Mcanglow(c) >= 1)
            && (Mdistance(c) < Mdistance(Ray.closest_mat))) {
                Ray.closest_mat = c;
            }
            if(Mdistance(c) <= HIT_DISTANCE) {
                Ray.hit = 1;
                Ray.mat = c;

                if(MreflectN(c) > 0.0) {
                    _FLAG_reflect = 1;
                    break;
                }
                else
                if(Mopaque(c) < 1.0) {
                    Ray.mat = c;
                    ray_outside = 0;
                }
                else {
                    break;
                }
            }

            Ray.len += Mdistance(c);
        }
        else {
            Ray.pos = ro + rd * (Ray.len + Ray.vm_len);

            Material c = map(Ray.pos);
            if(Mdistance(c) >= HIT_DISTANCE+0.01) {
                vm_map();
                Ray.mat = c;
                Ray.len += Ray.vm_len;
                ray_outside = 1;
            }
            Ray.vm_len += 0.1;
        }       
    }
}
FUNC_END

/* -INFO
   test
*/
FUNC void Raymarch(vec3 ro, vec3 rd)
{
    _FLAG_reflect = 0;
    Ray.closest_mat = EmptyMaterial();
    Raymarch_I(ro, rd);
    
    if(_FLAG_reflect == 1) {
        Ray.reflect_len = 0.0;
        vec3 new_rd = normalize(reflect(rd, ComputeNormal(Ray.pos)));
        vec3 new_ro = Ray.pos + (new_rd + HIT_DISTANCE+0.01);
        
        float rl = Ray.len;
        Ray.reflectoff_mat = Ray.mat;
        Raymarch_I(new_ro, new_rd);

        if(Ray.hit == 1) {
            Mdiffuse(Ray.mat) = mix(Mdiffuse(Ray.reflectoff_mat), Mdiffuse(Ray.mat), 0.5);
        }
        Ray.reflect_len = Ray.len;
        Ray.len = rl;
    }
}
FUNC_END


/* -INFO
Results can be accessed from Ray (RAY_T) struct.
   - ro is the ray origin position.
   - rd is the ray direction.
   - rd must be normalized.
   - user must define the map function.
*/
/*
FUNC void Raymarch_DISABLED(vec3 ro, vec3 rd)
{
    Ray.hit = 0;
    Ray.len = 0.0;
    Ray.vm_len = 0.0;
    Ray.pos = ro;
    Ray.closest_mat = Material(0);
    Ray.reflectoff_mat = Material(0);
    Ray.mat = Material(0);
    
    int ray_outside = 1;

    while(Ray.len < MAX_RAY_LENGTH) {

        if(ray_outside == 1) {
            Ray.pos = ro + rd * Ray.len;
            Material c = map(Ray.pos);
            if(Mdistance(c) <= HIT_DISTANCE) {
                Ray.hit = 1;
                Ray.mat = c;

                if(MreflectN(c) > 0.0) {
                    Ray.reflectoff_mat = Ray.mat;
                    vec3 new_rd = normalize(reflect(rd, ComputeNormal(Ray.pos)));
                    vec3 new_ro = Ray.pos + (new_rd + HIT_DISTANCE+0.01);
                    _Iraymarch_reflect(new_ro, new_rd);
                    break;
                }
                else
                if(Mopaque(c) < 1.0) {
                    Ray.mat = c;
                    ray_outside = 0;
                }
                else {
                    break;
                }
            }

            Ray.len += Mdistance(c);
        }
        else {
            Ray.pos = ro + rd * (Ray.len + Ray.vm_len);

            Material c = map(Ray.pos);
            if(Mdistance(c) >= HIT_DISTANCE+0.01) {
                vm_map();
                Ray.mat = c;
                Ray.len += Ray.vm_len;
                ray_outside = 1;
            }
            Ray.vm_len += 0.1;
        }
    }
}
FUNC_END
*/

/* -INFO
Returns material which distance is smaller.
*/
FUNC Material MaterialMin(Material a, Material b)
{
    return (Mdistance(a) < Mdistance(b)) ? a : b;
}
FUNC_END

/* -INFO
Returns material which distance is larger.
*/
FUNC Material MaterialMax(Material a, Material b)
{
    return (Mdistance(a) > Mdistance(b)) ? a : b;
}
FUNC_END

/* -INFO
Mix two materials.
   - t is the interpolation.
*/
FUNC Material MixMaterial(Material a, Material b, float t)
{
    Material m = Material(0);
    Mdiffuse(m) = mix(Mdiffuse(a), Mdiffuse(b), t);
    Mspecular(m) = mix(Mspecular(a), Mspecular(b), t);
    Mdistance(m) = mix(Mdistance(a), Mdistance(b), t);
    return m;
}
FUNC_END

/* -INFO
   - k is the strength.
*/
FUNC Material SmoothMixMaterial(Material a, Material b, float k)
{
    float d1 = Mdistance(a);
    float d2 = Mdistance(b);

    float t = clamp(0.5+0.5 * (d2 - d1) / k, 0.0, 1.0);
    Material m = MixMaterial(b, a, t);
    Mdistance(m) -= k * t * (1.0 - t);
    return m;
}
FUNC_END


/* -INFO
Calculate light values for the material 'm'
   - Ambient color is _not_ set by this function.
   - Material must have Mspecular and Mdiffuse set correctly.
*/
FUNC vec3 LightDirectional(vec3 eye, vec3 direction, vec3 color, vec3 normal, Material m)
{
    
    vec3 view_dir = normalize(eye - Ray.pos);
    vec3 halfway_dir = normalize(direction - view_dir);

    float nh_dot = max(dot(normal, halfway_dir), 0.0);
    float shine = 5.0; // TODO: Create input for this.

    vec3 specular = color + pow(nh_dot, shine) * Mspecular(m);
    float diffuse = max(dot(normal, direction), 0.0);

    return (specular * diffuse) * Mdiffuse(m);
}
FUNC_END

/* -INFO
Calculate light values for the material 'm'
   - Ambient color is _not_ set by this function.
   - Material must have Mspecular and Mdiffuse set correctly.
*/
FUNC vec3 LightPoint(vec3 eye, vec3 pos, vec3 color, float radius, float att, vec3 normal, Material m)
{
    vec3 light_dir = normalize(Ray.pos - pos);
    vec3 view_dir = normalize(eye - Ray.pos);
    vec3 halfway_dir = normalize(light_dir - view_dir);


    float n_dist = clamp(distance(pos, Ray.pos), 0, radius) / radius;
    
    n_dist = pow(n_dist, att);
    float dist = 1.0 / (1.0 + n_dist*n_dist);

    float nh_dot = max(dot(normal, halfway_dir), 0.0);
    float shine = 5.0; // TODO: Create input for this.

    vec3 specular = color + pow(nh_dot, shine) * (Mspecular(m) * Mdiffuse(m));
    float diffuse = max(dot(normal, light_dir), 0.0);

    specular *= dist;
    diffuse *= dist;

    return (specular * diffuse) * Mdiffuse(m);
}
FUNC_END

/* -INFO
https://iquilezles.org/articles/palettes/
Example: vec3 color = Palette(time, RAINBOW_PALETTE);
*/
FUNC vec3 Palette(float t, vec3 a, vec3 b, vec3 c, vec3 d)
{
    return a + b * cos(PI2 * (c * t + d));
}
FUNC_END

/* -INFO
https://iquilezles.org/articles/fog/
*/
FUNC vec3 ApplyFog(vec3 current_color, float density, vec3 fog_color)
{
    if(MreflectN(Ray.reflectoff_mat) > 0) { /* Shape is reflective */

        // Fog has to be applied to the reflection ray
        // and the shape itself.

        // Mix fog and the shape color which the ray reflected off.
        vec3 fog_color_refl = mix(Mdiffuse(Ray.reflectoff_mat), fog_color, 0.5);
        float fog_amount_refl = 1.0 - exp(-(Ray.reflect_len*0.01) * density);
        
        vec3 color = mix(current_color, fog_color_refl, fog_amount_refl);

        float fog_amount = 1.0 - exp(-(Ray.len*0.01) * density);
        return mix(color, fog_color, fog_amount);
    }
    else { /* Non-reflective */
        float fog_amount = 1.0 - exp(-(Ray.len*0.01) * density);
        return mix(current_color, fog_color, fog_amount);
    }

    return current_color;
}
FUNC_END

/* -INFO
Returns a pseudo random 2D vector.
*/
FUNC vec2 Hash2(vec2 x)
{
    return fract(sin(
        vec2(dot(x, vec2(95.32, 12.58)),
             dot(x, vec2(85.23, 32.23)))
        ) * 43758.5453);
}
FUNC_END

/* -INFO
Returns a pseudo random 3D vector.
*/
FUNC vec3 Hash3(vec3 x)
{
    return fract(sin(
        vec3(dot(x, vec3(95.32, 12.58, 23.23)),
             dot(x, vec3(85.23, 32.23, 13.56)),
             dot(x, vec3(78.32, 53.32, 73.32)))
        ) * 43758.5453);
}
FUNC_END

/* -INFO
https://iquilezles.org/articles/smoothvoronoi/
*/
FUNC float SmoothVoronoi2D(vec2 x, float falloff, float k)
{
   vec2 p = floor(x);
   vec2 f = fract(x);
   float res = 0.0;
   for(int y = -1; y <= 1; y++) {
      for(int x = -1; x <= 1; x++) {
         vec2 b = vec2(float(x), float(y));
         vec2 r = vec2(b) - f + Hash2(p + b);
         float d = dot(r, r);
         res += 1.0 / pow(d, k);
      }
   }
   return pow(1.0/res, 1.0/falloff);
}
FUNC_END

/* -INFO
https://iquilezles.org/articles/smoothvoronoi/
Expanded into 3D.
*/
FUNC float SmoothVoronoi3D(vec3 x, float falloff, float k)
{
   vec3 p = floor(x);
   vec3 f = fract(x);
   vec3 res = vec3(0.0);

   for(int z = -1; z <= 1; z++) {
      for(int y = -1; y <= 1; y++) {
         for(int x = -1; x <= 1; x++) {
            vec3 b = vec3(float(x), float(y), float(z));
            vec3 r = vec3(b) - f + Hash3(p + b);
            float d = dot(r, r);
            res += 1.0/pow(d, k);
         }
      }
   }
   return pow(1.0/res.x, 1.0/falloff);
}
FUNC_END

/* -INFO
https://iquilezles.org/articles/sdfrepetition/
Repeat space infinite
*/
FUNC vec3 RepeatINF(vec3 p, vec3 s)
{
    return p - s * round(p / s);
}
FUNC_END

/* -INFO
https://iquilezles.org/articles/sdfrepetition/
Repeat space limited
*/
FUNC vec3 RepeatLIM(vec3 p, vec3 s, vec3 lim)
{
    return p - s * clamp(round(p / s), -lim, lim);
}
FUNC_END


// ----- Signed Distance Functions -----

/* -INFO
https://iquilezles.org/articles/distfunctions/
Signed distance to a sphere.
*/
FUNC float SphereSDF(vec3 p, float radius)
{
    return length(p) - radius;
}
FUNC_END

/* -INFO
https://iquilezles.org/articles/distfunctions/
Signed distance to box/cube.
*/
FUNC float BoxSDF(vec3 p, vec3 size)
{
    vec3 q = abs(p) - size;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y,q .z)), 0.0);
}
FUNC_END


/* -INFO
https://iquilezles.org/articles/distfunctions/
Signed distance to box frame.
   - e is the frame size.
*/
FUNC float BoxFrameSDF(vec3 p, vec3 size, float e)
{
    p = abs(p)-size;
    vec3 q = abs(p+e)-e;
    return min(min(
        length(max(vec3(p.x,q.y,q.z),0.0))+min(max(p.x,max(q.y,q.z)),0.0),
        length(max(vec3(q.x,p.y,q.z),0.0))+min(max(q.x,max(p.y,q.z)),0.0)),
        length(max(vec3(q.x,q.y,p.z),0.0))+min(max(q.x,max(q.y,p.z)),0.0));
}
FUNC_END


/* -INFO
https://iquilezles.org/articles/distfunctions/
Signed distance to torus.
*/
FUNC float TorusSDF(vec3 p, vec2 t)
{
    vec2 q = vec2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}
FUNC_END


/* -INFO
https://iquilezles.org/articles/distfunctions/
Signed distance to line.
   - h is the line height.
   - r is the radius/width.
*/
FUNC float LineSDF(vec3 p, float h, float r)
{
    p.y -= clamp(p.y, 0.0, h);
    return length(p) - r;
}
FUNC_END



/* -INFO
https://iquilezles.org/articles/distfunctions/
Signed distance to cylinder
   - h is the height
   - r is the radius/width
*/
FUNC float CylinderSDF(vec3 p, float h, float r)
{
    vec2 d = abs(vec2(length(p.xz),p.y)) - vec2(r,h);
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}
FUNC_END




















