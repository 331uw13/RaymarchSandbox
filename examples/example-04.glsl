
// Example #4
//
// Simple terrain generation with reflective water.
// Try increasing the maximum ray length for better results


Material map(vec3 p) {
    p.z += time*300;
    vec3 q = p; // Save current ray pos before modifying it for later.
    
    vec3 ground_color = vec3(1.0, 0.9, 0.8);
    Material ground = EmptyMaterial();
    
    float path = smoothstep(0, 1, abs(p.x*0.01));
    path = clamp(path, 0, 1);
   
    float very_lowfreq = PerlinNoise2D(p.xz * 0.002) * (90.0 * path);
    float lowfreq      = PerlinNoise2D(p.xz * 0.015) * 15;
    float midfreq      = PerlinNoise2D(p.xz * 0.03) * 8;
    
    very_lowfreq *= 2;
    p.y += very_lowfreq + lowfreq + midfreq;
   
    // +100 to avoid gap in the middle.
    float raymax_X = abs(p.x*5.0)+100;
    float raymax_Z = abs(p.z*5.0)+100;
    vec3 size = vec3(raymax_X, 40, raymax_Z);
    Mdistance(ground) = BoxSDF(p, size);
    Mdiffuse(ground) = mix(
        vec3(0.0, 1.0, 0.3), // Green
        vec3(1.0, 0.3, 0.5), // Pink
        clamp(q.y * 0.01 + midfreq*0.02, 0, 1)
        );
    
    float water_level = 20.0;
    
    vec3 water_pos = q + vec3(0, water_level, 0);
    Material water = EmptyMaterial();
    Mdistance(water) = BoxSDF(water_pos, size);
    Mdiffuse(water) = vec3(0.2, 0.5, 1.0);
    MreflectN(water) = 1;
    
    
    return MaterialMin(water, ground);
}

// Not used but needs to be defined.
void vm_map() {} 


void main() {
    vec3 out_color = vec3(0.02);
    vec3 eye = vec3(0, 130, -8);
    vec3 rd = Raydir();
    
    rd.zy *= RotateM2(-0.3);
    
    // See "struct RAY_T" from function tabble on the right -->
    // for more information about
    // the results Raymarch function will give.
    Raymarch(eye, rd);
    
    if(Ray.hit == 1) {
        vec3 surface_normal = ComputeNormal(Ray.pos);
        
        //Mdiffuse(Ray.mat) = Palette(Ray.pos.y*0.005, RAINBOW_PALETTE);

        // Light has to be created for the scene.
        vec3 light_direction = vec3(3, 5, -3);
        vec3 light_color = vec3(1.0, 0.9, 0.8);
        float light_brightness = 0.1;
        
        vec3 light = LightDirectional(
            eye,
            light_direction,
            light_color,
            surface_normal,
            Ray.mat // The material which the ray hit.
        );
        light *= light_brightness;
        
        vec3 ambient = Mdiffuse(Ray.mat) * 0.05;
        out_color = light + ambient;
    }
    
    // Map fog density to max ray length.
    float fog_density = 1.0 - clamp(MAX_RAY_LENGTH, 0, 2350) / 3000;
    vec3  fog_color = vec3(0.4, 0.5, 0.5);
    float fog_exp = 8.0;
    
    out_color = ApplyFog(out_color, fog_density, fog_color, fog_exp);

    SetPixel(out_color);
}

