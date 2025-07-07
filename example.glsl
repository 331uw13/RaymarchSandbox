
vec3 light_pos = vec3(cos(time)*8.0, 1.0, sin(time)*8.0+8);

Material map(vec3 p) {

    p.z -= 15.0;
    p.y += 5;
    // GROUND
   
    float pattern = floor(sin(p.x)*2.0+sin(p.z)*2.0)+PI*2;
    Material ground = EmptyMaterial();
    Mdistance(ground) = BoxSDF(p, vec3(500.0, 0.2, 500.0));
    Mdiffuse(ground) = vec3(1.0, 1.0, 1.0) * pattern*0.05; 
    Mspecular(ground) = vec3(1.0);
        
    // REFLECTIVE SHAPE
    //float vn = SmoothVoronoi3D(p, 8, 16);
    vec3 sphere1_pos = p + vec3(1.0, -8.0, -2.0);
    Material sphere1 = EmptyMaterial();
    Mdistance(sphere1) = SphereSDF(sphere1_pos, 2.0);
    Mdiffuse(sphere1) = test_color.rgb;
    Mspecular(sphere1) = vec3(3.0);
    MreflectN(sphere1) = 1;
    
    // TRANSPARENT BOX FRAME
    vec3 tbox_pos = p + vec3(3.0, -5.0, 2.0);
    tbox_pos *= RotateM3(vec2(time*0.2, time*0.5));
    Material tbox = EmptyMaterial();
    Mdistance(tbox) = BoxFrameSDF(tbox_pos, vec3(1.5), 0.2)-0.1;
    Mdiffuse(tbox) = transparent_color.rgb;
    Mopaque(tbox) = 0;
    Mcanglow(tbox) = 1;
    
    // TRANSPARENT TORUS
    vec3 torus_pos = p + vec3(-3.0, -5.0, 2.0);
    torus_pos *= RotateM3(vec2(time*0.2, time*0.5));
    Material torus = EmptyMaterial();
    Mdistance(torus) = TorusSDF(torus_pos, vec2(1.5, 0.5))-0.1;
    Mdiffuse(torus) = Palette(length(torus_pos*0.5), RAINBOW_PALETTE);
    Mopaque(torus) = 0;
    Mcanglow(torus) = 1;
    
    // GRID
    p.y += 2.0;
    Material grid = EmptyMaterial();
    vec3 q = RepeatINF(p+10, vec3(13.0));
    Mdistance(grid) = BoxFrameSDF(q, vec3(2.0), 0.5);
    Mdiffuse(grid) = vec3(0.2);
    
    Material 
    result = ground;
    result = MaterialMin(result, tbox);
    result = MaterialMin(result, torus);
    result = MaterialMin(result, sphere1);
    result = MaterialMin(result, grid);
    
    return result;
}

void vm_map() {
    float len = pow(Ray.vm_len, 2);
    float density = 1.0 / (len  * 10.0);
    float vn = SmoothVoronoi3D(Ray.pos*0.6+vec3(0,time*0.25,0), 8, 10);
    density += vn*2.3;
    Ray.volume_color += Mdiffuse(Ray.mat) * density * 0.5;
    
}

void main() {
    vec3 out_color = vec3(0, 0, 0);
    
    vec3 rd = CameraInputRotation(Raydir());
    vec3 eye = CameraInputPosition;
    
    Ray.volume_color = vec3(0);
    
    Raymarch(eye, rd);
    if(Ray.hit == 1) {
    
        
        // Color solid materials.
            
        vec3 normal = ComputeNormal(Ray.pos);
        vec3 sun_light = LightDirectional(
            eye,
            vec3(-0.5, -1.0, 0.5), // Direction
            vec3(1.0, 0.9, 0.8), // Color
            normal,
            Ray.mat
        );
        
        vec3 point_light = LightPoint(
            eye,
            light_pos, // Position
            light_color.rgb, // Color
            7.0,
            3.0,
            normal,
            Ray.mat
        );
        
        vec3 ambient = Mdiffuse(Ray.mat) * 0.1;
        out_color = sun_light*0.6 + point_light + ambient;
        
    }
    
    out_color = ApplyFog(out_color, 1.0, fog_color.rgb);
    
    
    float glow = clamp(Mdistance(Ray.closest_mat), 0, 1);
    glow = 1.0 - glow * 3.0;
    glow = clamp(glow, 0, 1);
    out_color += Mdiffuse(Ray.closest_mat) * (glow * 0.2);
    
     
    SetPixel(out_color);
}



// This will add custom uniforms automatically
// when the shader is compiled the first time.
// They can be then modified at run time
// from "Input" tab. Or add new ones from the gui.
@startup_cmd

ADD COLOR test_color (0.844, 0.414, 0.972, 1.000);
ADD COLOR light_color (0.329, 0.062, 0.398, 0.882);
ADD COLOR fog_color (0.248, 0.791, 0.614, 0.000);
ADD COLOR transparent_color (0.303, 0.867, 0.819, 1.000);
ADD VALUE test_value (0.039, 0.000, 0.000, 0.000);

@end

