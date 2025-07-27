/*
  Example #7
    
    Mandelbulb Fractal.
*/



float MandelbulbSDF(vec3 P, float s) {
    float res = MAX_RAY_LENGTH;
    
    P *= s; // "global" scale for result.
    
    vec3 z = P;        // Current point.
    float dr = 1.0;    // Derivative accumulator
    float power = 10.0;
    
    float r;
    
    // Per iteration:
    // Convert 'z' to spherical coordinates
    // then update derivative accumulator,
    // apply transformation and scaling
    // after that convert back to cartesian coordinates.
    
    for(int i = 0; i < 4; i++) {
        
        // To Spherical coordinates
        r = length(z);
        float theta = acos(z.z / r);
        float phi = atan(z.y, z.x);
        if(r > 3.0) { break; }
        
        // Scaling.
        dr = dr * power * pow(r, power - 1.0) + 1.0;
        r = pow(r, power);
        theta *= power;
        phi *= power;
        
        // Back to Cartesian coordinates.
        z.x = r * sin(theta) * cos(phi);
        z.y = r * sin(theta) * sin(phi);
        z.z = r * cos(theta);
    
        z += P;
    }
    
    // Distance estimation.
    res = 0.5 * log(r) * (r / dr);
    return res / s;
}



// This function will map all the materials for the scene.
Material map(vec3 p) {

    p.z -= 70.0;
    
    float t = time*0.2;
    p *= RotateM3(vec2(t*0.5, t));
    
    Material m = EmptyMaterial();
    Mdistance(m) = MandelbulbSDF(p, 0.04);
    Mdiffuse(m) = Palette(length(p)*0.05+time*0.2, RAINBOW_PALETTE);

    return m;
}

vec3 raycolor_translucent() {
    float density = Ray.vm_len*0.1;
    
    return density * Mdiffuse(Ray.mat);
}

vec3 raycolor() {
    vec3 color = vec3(0);
    
    vec3 normal = ComputeNormal(Ray.pos);
    float light_strength = 2.0;
    vec3 light_pos = 
        vec3(8.0, 20.0, -3.0);
        
    vec3 light = LightDirectional(
        CameraInputPosition,
        light_pos,
        vec3(1.0, 0.9, 0.8),
        normal,
        Ray.mat
    ) * light_strength;
    vec3 ambient = Mdiffuse(Ray.mat) * 0.1;
    color = light + ambient;
    
    color *= GetShadow_LightPoint(Ray.pos, light_pos, 0.5, 0.1);
    return color;
}


// This function will get called right after
// initializing RAY_T struct in main().
void entry() {

    FOG_DENSITY = 2.0;
    FOG_EXPONENT = 5.0;
    FOG_COLOR = fog_color.rgb;
    
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

ADD COLOR fog_color (0.447, 1.000, 0.954, 1.000);

@end

