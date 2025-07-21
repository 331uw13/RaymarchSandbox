/*
  Example #1
    
    Hello Sphere.
*/


// This function will map all the materials for the scene.
Material map(vec3 p) {
    
    p.z -= 5.0;
    
    // Try changing the sphere color from the "Uniform Input" tab.
    Material sphere = EmptyMaterial();
    Mdistance(sphere) = SphereSDF(p, 1.0);
    Mdiffuse(sphere) = sphere_color.rgb;

    return sphere;
}


NO_TRANSLUCENT_COLORS

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
    
    return light + ambient;
}


// This function will get called right after
// initializing RAY_T struct in main().
void entry() {
    
    vec3 rd = CameraInputRotation(Raydir());
    vec3 eye = CameraInputPosition;
    
    Raymarch(eye, rd);
    
    // Write results to the output texture.
    
    SetPixel(GetFinalColor());
}



// This will add custom uniforms automatically
// when the shader is compiled the first time.
// They can be then modified at run time
// from "Input" tab. Or add new ones from the gui.
@startup_cmd

ADD COLOR sphere_color (0.928, 0.583, 0.387, 1.000);

@end

