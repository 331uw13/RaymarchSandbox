

// https://github.com/331uw13/RaymarchSandbox


layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout (rgba16f, binding = 8) uniform image2D output_img;

uniform vec2 monitor_size;
uniform float time;
uniform float FOV;
uniform float HIT_DISTANCE;
uniform float MAX_RAY_LENGTH;
uniform float TRANSLUCENT_STEP_SIZE;
uniform float CAMERA_INPUT_YAW;
uniform float CAMERA_INPUT_PITCH;
uniform vec3 CameraInputPosition;
uniform float AO_STEP_SIZE;
uniform int AO_NUM_SAMPLES;
uniform float AO_FALLOFF;

#define PI 3.14159
#define PI2 (PI*2.0)
#define PI_R (PI/180.0)
#define ColorRGB(r,g,b) vec3(r/255.0, g/255.0, b/255.0)

#define RAINBOW_PALETTE vec3(0.5, 0.5, 0.5), vec3(0.5, 0.5, 0.5),vec3(1.0, 1.0, 1.0), vec3(0.0, 0.33, 0.67)

#define NO_TRANSLUCENT_COLORS\
    vec3 raycolor_translucent(){ return vec3(0); }


#define Material mat4x3
#define Mdiffuse(x)     x[0]
#define Mspecular(x)    x[1]
#define Mdistance(x)    x[2][0]
#define MreflectN(x)    x[2][1]
#define Mopaque(x)      x[2][2]
#define Mcanglow(x)     x[3][0]


vec3 FOG_COLOR = vec3(0.5, 0.5, 0.5);
float FOG_DENSITY = 1.0;
float FOG_EXPONENT = 2.0;


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
   - Ray.mat      : Material is being processed.
   - Ray.vm_len   : Distance from enter point to exit point.
*/
FUNC vec3 raycolor_translucent();
FUNC_END


/* -INFO
This function is called when the ray needs a color.
*/
FUNC vec3 raycolor();
FUNC_END


/* -INFO
Raymarch function will set these variables.
RAY_T Ray;
*/
struct RAY_T
{
    vec3  solid_color;
    vec3  volume_color;
    float diffuse_value;
    float alpha;
    int   hit;
    vec3  pos;
    float first_hit_dist;
    float len;    // Ray length to first ray hit (reflection doesnt use this.)
    float vm_len; // Distance from enter point to exit point.
        
    Material mat; // Material which ray hit.
    Material closest_mat; // Closest material to ray.

    float reflect_len; // Ray length after hit to reflective material.
};
RAY_T Ray;



void entry();
void main() {
    Ray.volume_color = vec3(0, 0, 0);
    Ray.hit = 0;
    Ray.len = 0;
    Ray.first_hit_dist = -1.0;
    Ray.vm_len = 0;
    Ray.mat = Material(0);
    Ray.closest_mat = Material(0);
    Ray.reflect_len = 0;
    entry();
}


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


float GetFogFactor(float raylen) {
    return 1.0 - exp(-(raylen*0.01) * FOG_DENSITY);
}

// https://iquilezles.org/articles/fog/
vec3 ApplyFog(vec3 current_color, float raylen)
{
    return mix(current_color, FOG_COLOR*0.5, pow(GetFogFactor(raylen), FOG_EXPONENT));
}

/* -INFO
*/
FUNC vec3 GetFinalColor()
{
    if(Ray.first_hit_dist < 0) {
        Ray.first_hit_dist = Ray.len;
    }

    vec3 
    ray_color = ApplyFog(Ray.solid_color, Ray.len);
    ray_color += (Ray.volume_color) * (1.0-GetFogFactor(Ray.first_hit_dist));
    
    return pow(ray_color, vec3(1.0/1.6));
}
FUNC_END

/* -INFO
Set color for the current pixel.
*/
FUNC void SetPixel(vec3 color)
{
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
    vec2 e = vec2(0.0005, 0.0);
    return normalize(vec3(
        Mdistance(map(p - e.xyy)) - Mdistance(map(p + e.xyy)),
        Mdistance(map(p - e.yxy)) - Mdistance(map(p + e.yxy)),
        Mdistance(map(p - e.yyx)) - Mdistance(map(p + e.yyx))
    ));
}
FUNC_END

/* -INFO
Linear interpolation function.
*/
FUNC float Lerp(float t, float min, float max)
{
    return (max - min) * t + min;
}
FUNC_END

/* -INFO
Interpolate between 2 3D vectors.
*/
FUNC vec3 Vec3Lerp(float t, vec3 a, vec3 b)
{
    return vec3(
            Lerp(t, a.x, b.x),
            Lerp(t, a.y, b.y),
            Lerp(t, a.z, b.z)
            );
}
FUNC_END

int _FLAG_reflect = 0;
void Raymarch_I(vec3 ro, vec3 rd);

/* -INFO
Results can be accessed from Ray (RAY_T) struct.
   - ro is the ray origin position.
   - rd is the ray direction.
   - rd must be normalized.
   - user must define the map function.
*/
FUNC void Raymarch(vec3 ro, vec3 rd)
{
    _FLAG_reflect = 0;
    Ray.closest_mat = EmptyMaterial();
    
    Ray.volume_color = vec3(0);
    Ray.solid_color = vec3(0);

    Raymarch_I(ro, rd);
    
    if(_FLAG_reflect == 1) {
        vec3 normal = ComputeNormal(Ray.pos);
        vec3 new_rd = normalize(reflect(rd, normal));
        vec3 new_ro = Ray.pos + (new_rd + HIT_DISTANCE*0.01);

        RAY_T Oray = Ray; // Original ray.

        Raymarch_I(new_ro, new_rd);
        RAY_T rayR = Ray; // Reflected ray.
        Ray = Oray;
      
        Ray.solid_color = clamp(Ray.solid_color, vec3(0), vec3(1));
        rayR.solid_color = clamp(rayR.solid_color, vec3(0), vec3(1));
       
        vec3 view_dir = normalize(Ray.pos - ro);
        float R = (Ray.diffuse_value + 0.25) * MreflectN(Ray.mat);
        Ray.solid_color = Vec3Lerp(R, Ray.solid_color, rayR.solid_color);
        Ray.volume_color += Vec3Lerp(R, vec3(0), rayR.volume_color);
   
    }
}
FUNC_END

// FIXME: reflection ray will go into the material itself
//        if they are very close to each other.

/* -INFO
Reflections are not handled by this function.
Use Raymarch(...) instead.
*/
FUNC void Raymarch_I(vec3 ro, vec3 rd)
{    
    Ray.hit = 0;
    Ray.len = 0.0;
    Ray.vm_len = 0.0;
    Ray.pos = ro;
    Ray.mat = Material(0);
    Ray.diffuse_value = 0.0; 
    Ray.volume_color = vec3(0);
    Ray.first_hit_dist = -1;
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

                if(Ray.first_hit_dist < 0) {
                    Ray.first_hit_dist = Ray.len;
                }

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
                Ray.volume_color += raycolor_translucent();
                Ray.volume_color = clamp(Ray.volume_color, vec3(0), vec3(1));
                Ray.mat = c;
                Ray.len += Ray.vm_len;
                ray_outside = 1;
            }
            Ray.vm_len += TRANSLUCENT_STEP_SIZE;
        }       
    }

    Ray.solid_color += ApplyFog(raycolor(), Ray.len);
    //Ray.solid_color = ApplyFog(Ray.solid_color, Ray.len);
}
FUNC_END


float MapValue(float t, float src_min, float src_max, float dst_min, float dst_max);

/* -INFO
Return a shadow value for point 'p'
to light point.
https://iquilezles.org/articles/rmshadows/
*/
FUNC float GetShadow_LightPoint(vec3 p, vec3 light_pos, float w, float max_value)
{
    w = MapValue(w, 0.0, 1.0, 0.001, 0.1);
    float shadow = 1.0;
    RAY_T old_ray = Ray;

    vec3 dir = normalize(light_pos - p);
    p += (dir * 0.01);
 
    Ray.hit = 0;
    Ray.len = 0.0;
    Ray.vm_len = 0.0;
    Ray.pos = p;
    Ray.mat = Material(0);

    float ph = 1e20;

    while(Ray.len < MAX_RAY_LENGTH) {
        Ray.pos = p + dir * Ray.len;
        Material c = map(Ray.pos);

        if(Mdistance(c) <= 0.0001) {
            shadow = 0.0;
            break;
        }

        float h = Mdistance(c);
        float y = h * h / (2.0 * ph);
        float d = sqrt(h*h - y*y);
        shadow = min(shadow, d/(w*max(0.0, Ray.len-y)));

        ph = h;

        Ray.len += Mdistance(c);
    }
    Ray = old_ray;
    return clamp(shadow, max_value, 1.0);
}
FUNC_END


vec3 Hash3(vec3 x);

/* -INFO
Returns ambient occlusion value for point p (current ray position)
*/
FUNC float AmbientOcclusion(vec3 p, vec3 normal)
{
    RAY_T old_ray = Ray; // Save ray if user modifies it from map function.
    float ao = 0.0;

    for(int i = 1; i <= AO_NUM_SAMPLES; i++) {

        // Get random vector and normalize it to -1.0 to 1.0.
        // Time is added to reduce noise visible to human eye.
        vec3 rV = Hash3(p + i + time*2) * 2.0 - 1.0;

        // Decrease rV contribution to direction
        // as the distance increases.
        float t = float(i) / (float(AO_NUM_SAMPLES) * 2.0);
        t = pow(t, AO_FALLOFF);

        vec3 dir = (-normal) * (i * AO_STEP_SIZE) + (rV * t);
        float dist = Mdistance(map(dir + p));

        ao += max(dist, 0.0);
    }

    ao /= (float(AO_NUM_SAMPLES)*0.25);

    Ray = old_ray;
    return ao;
}
FUNC_END


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
    Material m = EmptyMaterial();
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
    vec3 light_dir = -normalize(direction);
    vec3 halfway_dir = normalize(light_dir - view_dir);
    
    // Diffuse.
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = diff * Mdiffuse(m);

    // Specular.
    float shine = 5.0;
    float spec = pow(max(dot(view_dir, halfway_dir), 0.0), shine);
    vec3 specular = spec * Mspecular(m);

    return mix(diffuse, specular, 0.5);
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

    // Diffuse.
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = diff * Mdiffuse(m);

    // Specular.
    float shine = 5.0;
    float spec = pow(max(dot(view_dir, halfway_dir), 0.0), shine);
    vec3 specular = spec * Mspecular(m);
    
    // Attenuation.
    float L = 0.7;
    float Q = 1.8;
    float dist = clamp(distance(pos, Ray.pos), 0.0, radius) / radius;
    dist = pow(dist, att);
    float a = 1.0 / (2.0 + L * dist + Q * (dist * dist));

    Ray.diffuse_value = diff;

    diffuse *= a;
    specular *= a;

    return diffuse + specular;
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
Map value t from src range to dst range.
*/
FUNC float MapValue(float t, float src_min, float src_max, float dst_min, float dst_max)
{
    return (t - src_min) * (dst_max - dst_min) / (src_max - src_min) + dst_min;
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


vec2 _fade(vec2 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}
vec4 _permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}

/* -INFO
2D Perlin noise by Stefan Gustavson (https://github.com/stegu/webgl-noise)
*/
FUNC float PerlinNoise2D(vec2 P)
{
  vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
  vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
  Pi = mod(Pi, 289.0); // To avoid truncation effects in permutation
  vec4 ix = Pi.xzxz;
  vec4 iy = Pi.yyww;
  vec4 fx = Pf.xzxz;
  vec4 fy = Pf.yyww;
  vec4 i = _permute(_permute(ix) + iy);
  vec4 gx = 2.0 * fract(i * 0.0243902439) - 1.0; // 1/41 = 0.024...
  vec4 gy = abs(gx) - 0.5;
  vec4 tx = floor(gx + 0.5);
  gx = gx - tx;
  vec2 g00 = vec2(gx.x,gy.x);
  vec2 g10 = vec2(gx.y,gy.y);
  vec2 g01 = vec2(gx.z,gy.z);
  vec2 g11 = vec2(gx.w,gy.w);
  vec4 norm = 1.79284291400159 - 0.85373472095314 *
    vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11));
  g00 *= norm.x;
  g01 *= norm.y;
  g10 *= norm.z;
  g11 *= norm.w;
  float n00 = dot(g00, vec2(fx.x, fy.x));
  float n10 = dot(g10, vec2(fx.y, fy.y));
  float n01 = dot(g01, vec2(fx.z, fy.z));
  float n11 = dot(g11, vec2(fx.w, fy.w));
  vec2 fade_xy = _fade(Pf.xy);
  vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
  float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
  return 2.3 * n_xy;
}
FUNC_END

vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}
vec4 _fade(vec4 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}

/* -INFO
3D Perlin noise by Stefan Gustavson (https://github.com/stegu/webgl-noise)
*/
FUNC float PerlinNoise3D(vec3 p)
{
  vec4 P = vec4(p, 1.0);
  vec4 Pi0 = floor(P); // Integer part for indexing
  vec4 Pi1 = Pi0 + 1.0; // Integer part + 1
  Pi0 = mod(Pi0, 289.0);
  Pi1 = mod(Pi1, 289.0);
  vec4 Pf0 = fract(P); // Fractional part for interpolation
  vec4 Pf1 = Pf0 - 1.0; // Fractional part - 1.0
  vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
  vec4 iy = vec4(Pi0.yy, Pi1.yy);
  vec4 iz0 = vec4(Pi0.zzzz);
  vec4 iz1 = vec4(Pi1.zzzz);
  vec4 iw0 = vec4(Pi0.wwww);
  vec4 iw1 = vec4(Pi1.wwww);

  vec4 ixy = _permute(_permute(ix) + iy);
  vec4 ixy0 = _permute(ixy + iz0);
  vec4 ixy1 = _permute(ixy + iz1);
  vec4 ixy00 = _permute(ixy0 + iw0);
  vec4 ixy01 = _permute(ixy0 + iw1);
  vec4 ixy10 = _permute(ixy1 + iw0);
  vec4 ixy11 = _permute(ixy1 + iw1);

  vec4 gx00 = ixy00 / 7.0;
  vec4 gy00 = floor(gx00) / 7.0;
  vec4 gz00 = floor(gy00) / 6.0;
  gx00 = fract(gx00) - 0.5;
  gy00 = fract(gy00) - 0.5;
  gz00 = fract(gz00) - 0.5;
  vec4 gw00 = vec4(0.75) - abs(gx00) - abs(gy00) - abs(gz00);
  vec4 sw00 = step(gw00, vec4(0.0));
  gx00 -= sw00 * (step(0.0, gx00) - 0.5);
  gy00 -= sw00 * (step(0.0, gy00) - 0.5);

  vec4 gx01 = ixy01 / 7.0;
  vec4 gy01 = floor(gx01) / 7.0;
  vec4 gz01 = floor(gy01) / 6.0;
  gx01 = fract(gx01) - 0.5;
  gy01 = fract(gy01) - 0.5;
  gz01 = fract(gz01) - 0.5;
  vec4 gw01 = vec4(0.75) - abs(gx01) - abs(gy01) - abs(gz01);
  vec4 sw01 = step(gw01, vec4(0.0));
  gx01 -= sw01 * (step(0.0, gx01) - 0.5);
  gy01 -= sw01 * (step(0.0, gy01) - 0.5);

  vec4 gx10 = ixy10 / 7.0;
  vec4 gy10 = floor(gx10) / 7.0;
  vec4 gz10 = floor(gy10) / 6.0;
  gx10 = fract(gx10) - 0.5;
  gy10 = fract(gy10) - 0.5;
  gz10 = fract(gz10) - 0.5;
  vec4 gw10 = vec4(0.75) - abs(gx10) - abs(gy10) - abs(gz10);
  vec4 sw10 = step(gw10, vec4(0.0));
  gx10 -= sw10 * (step(0.0, gx10) - 0.5);
  gy10 -= sw10 * (step(0.0, gy10) - 0.5);

  vec4 gx11 = ixy11 / 7.0;
  vec4 gy11 = floor(gx11) / 7.0;
  vec4 gz11 = floor(gy11) / 6.0;
  gx11 = fract(gx11) - 0.5;
  gy11 = fract(gy11) - 0.5;
  gz11 = fract(gz11) - 0.5;
  vec4 gw11 = vec4(0.75) - abs(gx11) - abs(gy11) - abs(gz11);
  vec4 sw11 = step(gw11, vec4(0.0));
  gx11 -= sw11 * (step(0.0, gx11) - 0.5);
  gy11 -= sw11 * (step(0.0, gy11) - 0.5);

  vec4 g0000 = vec4(gx00.x,gy00.x,gz00.x,gw00.x);
  vec4 g1000 = vec4(gx00.y,gy00.y,gz00.y,gw00.y);
  vec4 g0100 = vec4(gx00.z,gy00.z,gz00.z,gw00.z);
  vec4 g1100 = vec4(gx00.w,gy00.w,gz00.w,gw00.w);
  vec4 g0010 = vec4(gx10.x,gy10.x,gz10.x,gw10.x);
  vec4 g1010 = vec4(gx10.y,gy10.y,gz10.y,gw10.y);
  vec4 g0110 = vec4(gx10.z,gy10.z,gz10.z,gw10.z);
  vec4 g1110 = vec4(gx10.w,gy10.w,gz10.w,gw10.w);
  vec4 g0001 = vec4(gx01.x,gy01.x,gz01.x,gw01.x);
  vec4 g1001 = vec4(gx01.y,gy01.y,gz01.y,gw01.y);
  vec4 g0101 = vec4(gx01.z,gy01.z,gz01.z,gw01.z);
  vec4 g1101 = vec4(gx01.w,gy01.w,gz01.w,gw01.w);
  vec4 g0011 = vec4(gx11.x,gy11.x,gz11.x,gw11.x);
  vec4 g1011 = vec4(gx11.y,gy11.y,gz11.y,gw11.y);
  vec4 g0111 = vec4(gx11.z,gy11.z,gz11.z,gw11.z);
  vec4 g1111 = vec4(gx11.w,gy11.w,gz11.w,gw11.w);

  vec4 norm00 = taylorInvSqrt(vec4(dot(g0000, g0000), dot(g0100, g0100), dot(g1000, g1000), dot(g1100, g1100)));
  g0000 *= norm00.x;
  g0100 *= norm00.y;
  g1000 *= norm00.z;
  g1100 *= norm00.w;

  vec4 norm01 = taylorInvSqrt(vec4(dot(g0001, g0001), dot(g0101, g0101), dot(g1001, g1001), dot(g1101, g1101)));
  g0001 *= norm01.x;
  g0101 *= norm01.y;
  g1001 *= norm01.z;
  g1101 *= norm01.w;

  vec4 norm10 = taylorInvSqrt(vec4(dot(g0010, g0010), dot(g0110, g0110), dot(g1010, g1010), dot(g1110, g1110)));
  g0010 *= norm10.x;
  g0110 *= norm10.y;
  g1010 *= norm10.z;
  g1110 *= norm10.w;

  vec4 norm11 = taylorInvSqrt(vec4(dot(g0011, g0011), dot(g0111, g0111), dot(g1011, g1011), dot(g1111, g1111)));
  g0011 *= norm11.x;
  g0111 *= norm11.y;
  g1011 *= norm11.z;
  g1111 *= norm11.w;

  float n0000 = dot(g0000, Pf0);
  float n1000 = dot(g1000, vec4(Pf1.x, Pf0.yzw));
  float n0100 = dot(g0100, vec4(Pf0.x, Pf1.y, Pf0.zw));
  float n1100 = dot(g1100, vec4(Pf1.xy, Pf0.zw));
  float n0010 = dot(g0010, vec4(Pf0.xy, Pf1.z, Pf0.w));
  float n1010 = dot(g1010, vec4(Pf1.x, Pf0.y, Pf1.z, Pf0.w));
  float n0110 = dot(g0110, vec4(Pf0.x, Pf1.yz, Pf0.w));
  float n1110 = dot(g1110, vec4(Pf1.xyz, Pf0.w));
  float n0001 = dot(g0001, vec4(Pf0.xyz, Pf1.w));
  float n1001 = dot(g1001, vec4(Pf1.x, Pf0.yz, Pf1.w));
  float n0101 = dot(g0101, vec4(Pf0.x, Pf1.y, Pf0.z, Pf1.w));
  float n1101 = dot(g1101, vec4(Pf1.xy, Pf0.z, Pf1.w));
  float n0011 = dot(g0011, vec4(Pf0.xy, Pf1.zw));
  float n1011 = dot(g1011, vec4(Pf1.x, Pf0.y, Pf1.zw));
  float n0111 = dot(g0111, vec4(Pf0.x, Pf1.yzw));
  float n1111 = dot(g1111, Pf1);

  vec4 fade_xyzw = _fade(Pf0);
  vec4 n_0w = mix(vec4(n0000, n1000, n0100, n1100), vec4(n0001, n1001, n0101, n1101), fade_xyzw.w);
  vec4 n_1w = mix(vec4(n0010, n1010, n0110, n1110), vec4(n0011, n1011, n0111, n1111), fade_xyzw.w);
  vec4 n_zw = mix(n_0w, n_1w, fade_xyzw.z);
  vec2 n_yzw = mix(n_zw.xy, n_zw.zw, fade_xyzw.y);
  float n_xyzw = mix(n_yzw.x, n_yzw.y, fade_xyzw.x);
  return 2.2 * n_xyzw;
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

/* -INFO
https://iquilezles.org/articles/distfunctions/
Signed distance to octahedron.
*/
FUNC float OctahedronSDF(vec3 p, float s)
{
    p = abs(p);
    float m = p.x+p.y+p.z-s;
    vec3 q;
    if( 3.0*p.x < m ) q = p.xyz;
        else if( 3.0*p.y < m ) q = p.yzx;
        else if( 3.0*p.z < m ) q = p.zxy;
        else return m*0.57735027;
    
    float k = clamp(0.5*(q.z-q.y+s),0.0,s); 
    return length(vec3(q.x,q.y-s+k,q.z-k)); 
}
FUNC_END

















