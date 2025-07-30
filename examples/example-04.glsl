/*
  Example #4
    
    Fog.


Note: Try changing values from the "Uniform Input" tab.
*/


// This function will map all the materials for the scene.
Material map(vec3 p) {
    
    p.z -= 8.0;
    
    
    vec3 sphere_pos = p + vec3(2.5, 0.0, -4.0);
    Material sphere = EmptyMaterial();
    Mdistance(sphere) = SphereSDF(sphere_pos, 2.0);
    Mdiffuse(sphere) = sphere_color.rgb;
    
    vec3 box_pos = p + vec3(1.0, -0.5, 0.0);
    box_pos *= RotateM3(vec2(time*0.2, time*0.5));
    Material box = EmptyMaterial();
    Mdistance(box) = BoxSDF(box_pos, vec3(1.0));
    Mdiffuse(box) = box_color.rgb;
    Mopaque(box) = 0;
    
    
    // Add ground to demonstrate the fog effect better.
    float ground_pattern = 
          floor(2*(sin(p.x*2.0)*0.5+0.5)) * 0.5
        + floor(2*(cos(p.z*2.0)*0.5+0.5)) * 0.5;
        ground_pattern += 0.25;
        
    Material ground = EmptyMaterial();
    Mdistance(ground) = BoxSDF(p+vec3(0, 5, 0), vec3(100, 0.1, 100));
    Mdiffuse(ground) = vec3(0.5, 0.8, 0.6) * ground_pattern;
    
    
    Material result = ground;
    result = MaterialMin(result, sphere);
    result = MaterialMin(result, box);
    
    return result;
}


vec3 raycolor_translucent() {
    // The vm_len is the distance between
    // Enter point and Exit point.
    float density = Ray.vm_len;
    
    // The distance can be inverted to give
    // to give the material a "hollow" effect.
    density = 1.0 / (density*5.0);
    
    return Mdiffuse(Ray.mat) * density;
}


vec3 raycolor() {
    vec3 color = vec3(0);
    
    vec3 normal = ComputeNormal(Ray.pos);
    float light_strength = 2.0;
    vec3 light = LightDirectional(
        CameraInputPosition,
        vec3(8.0, 10.0, -3.0),
        vec3(1.0, 0.9, 0.8),
        normal,
        Ray.mat
    ) * light_strength;
    
    vec3 ambient = Mdiffuse(Ray.mat) * 0.1;
    color = light + ambient;
    
    return color;
}


// This function will get called right after
// initializing RAY_T struct in main().
void entry() {

    FOG_COLOR = fog_colors.rgb;
    FOG_DENSITY = 4.0;
    FOG_EXPONENT = 3.0;
    
    vec3 rd = CameraInputRotation(Raydir());
    vec3 eye = CameraInputPosition;
    
    Raymarch(eye, rd);
    
    SetPixel(GetFinalColor());
}



// This will add custom uniforms automatically
// when the shader is compiled the first time.
// They can be then modified at run time
// from "Input" tab. Or add new ones from the gui.
@_UNIFORM_METADATA
    "sphere_color"(RGBA)[0.928, 0.583, 0.387, 1.000]
    "box_color"(RGBA)[0.137, 0.896, 0.850, 1.000]
    "fog_colors"(RGBA)[0.333, 0.474, 0.421, 1.000]
@_UNIFORM_METADATA_END
