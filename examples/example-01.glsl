
// Example #1
//
// Hello sphere.
//


Material map(vec3 p) {

    vec3 sphere_color = vec3(1.0, 0.9, 0.8);
    Material sphere = EmptyMaterial();
    Mdistance(sphere) = SphereSDF(p, 1.0);
    Mdiffuse(sphere) = sphere_color;
    Mspecular(sphere) = vec3(0.8);
    
    return sphere;
}

// Not used but needs to be defined.
void vm_map() {} 


void main() {
    vec3 out_color = vec3(0.02);
    vec3 eye = vec3(0, 0, -8);
    vec3 rd = Raydir();
    
    // See "struct RAY_T" from function tabble on the right -->
    // for more information about
    // the results Raymarch function will give.
    Raymarch(eye, rd);
    
    if(Ray.hit == 1) {
        vec3 surface_normal = ComputeNormal(Ray.pos);
   
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

    SetPixel(out_color);
}

