/*
  Example #6
    
    Soft shadows and Ambient occlusion.
    
          
Note: Try changing the colors from the "Uniform Input" tab.
*/


// This function will map all the materials for the scene.
Material map(vec3 p) {
    
    p.y -= 2.0;
    p.z -= 12.0;
    p.x += 2.0;
    
    float rb = time*0.05;
    float rb_i = 0.3;
  
    vec3 cyl_pos = p + vec3(3.5, 4.0, 0.0);
    cyl_pos *= RotateM3(vec2(PI/2, 0.5));
    Material cyl = EmptyMaterial();
    Mdistance(cyl) = CylinderSDF(cyl_pos, 2.0, 1.0);
    Mdiffuse(cyl) = Palette(rb, RAINBOW_PALETTE);
    rb += rb_i;
    
    vec3 box_pos = p + vec3(0.0, sin(time*1.3)*0.2+3.9, 0.0);
    box_pos.xz *= RotateM2(0.57);
    Material box = EmptyMaterial();
    Mdistance(box) = BoxSDF(box_pos, vec3(1.0));
    Mdiffuse(box) = box_color.rgb;
    MreflectN(box) = 0;
    rb += rb_i;
   
    vec3 torus_pos = p + vec3(-3.0, 3.0, 1.0);
    torus_pos *= RotateM3(vec2(-PI/3, -0.5));
    Material torus = EmptyMaterial();
    Mdistance(torus) = TorusSDF(torus_pos, vec2(1.0, 0.5));
    Mdiffuse(torus) = Palette(rb, RAINBOW_PALETTE);
    Mopaque(torus) = 0;
    rb += rb_i;
    

    float ground_pattern = 
          floor(2*(sin(p.x*2.0)*0.5+0.5)) * 0.03
        + floor(2*(cos(p.z*2.0)*0.5+0.5)) * 0.03;
        ground_pattern += 0.25;
        
    Material ground = EmptyMaterial();
    Mdistance(ground) = BoxSDF(p+vec3(0, 5, 0), vec3(100, 0.1, 100));
    Mdiffuse(ground) = vec3(0.8, 0.7, 0.5) * ground_pattern;
    MreflectN(ground) = 0;
    
    Material result = ground;
    result = MaterialMin(result, box);
    result = MaterialMin(result, cyl);
    result = MaterialMin(result, torus);
    return result;
}


vec3 raycolor_translucent() {

    float density = Ray.vm_len;
    density = density * density;
    density = 1.0 / (density*10.0);
    
    return Mdiffuse(Ray.mat) * density;
}

vec3 raycolor() {
    vec3 color = vec3(0);
    vec3 normal = ComputeNormal(Ray.pos);
    
    float light_strength = 1.0;
    vec3 light_pos = vec3(5.0, 12.0, -5.0 + 12);
    vec3 light = LightPoint(
        CameraInputPosition,
        light_pos,
        vec3(1.0, 0.9, 0.8),
        15.0, 1.0,
        normal,
        Ray.mat
    ) * light_strength;
    
    vec3 ambient = Mdiffuse(Ray.mat) * 0.1;
    color = Mdiffuse(Ray.mat);
    color = light + ambient;
    
    color *= GetShadow_LightPoint(Ray.pos, light_pos, 0.5, 0.3);
    
    int   ao_samples = 32;
    float ao_falloff = 3.0;
    color *= AmbientOcclusion(Ray.pos, normal);

    return color;
}


// This function will get called right after
// initializing RAY_T struct in main().
void entry() {
    
    FOG_COLOR = fog_colors.rgb;
    FOG_DENSITY = 5.0;
    FOG_EXPONENT = 5.0;
    
    vec3 rd = CameraInputRotation(Raydir());
    vec3 eye = CameraInputPosition;
    
    Raymarch(eye, rd);
    
    // Write results to the output texture.
    vec3 color = GetFinalColor();
    SetPixel(color);
}



// This will add custom uniforms automatically
// when the shader is compiled the first time.
// They can be then modified at run time
// from "Input" tab. Or add new ones from the gui.
@_UNIFORM_METADATA
    "fog_colors"(RGBA)[0.289, 0.289, 0.289, 1.000]
    "box_color"(RGBA)[0.275, 0.867, 0.703, 1.000]
@_UNIFORM_METADATA_END
