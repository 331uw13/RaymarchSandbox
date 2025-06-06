

Material map(vec3 p) {
    Material test = Material(0);

    Mdistance(test) = SPHERE_SDF(p, 0.5);
    Mdiffuse(test) = vec3(0.4, 0.8, 0.4);
    Mshine(test) = 3.0;
    Mspecular(test) = vec3(0.3);

    return test;
}



void main() {
    vec3 color = vec3(0);

    CAMERA.pos = vec3(0, 0, -8.0);
    CAMERA.dir = RAYDIR(); 


    RAYMARCH(CAMERA.pos, CAMERA.dir);

    if(RAY_RESULT.hit == 1) {
        vec3 normal = COMPUTE_NORMAL(RAY_RESULT.pos);
        vec3 light = COMPUTE_LIGHT(
                vec3(cos(time*3.0)*8.0, cos(time)*10.0, sin(time)*4.0),
                vec3(0.8, 0.7, 0.5),
                normal,
                RAY_RESULT.pos,
                RAY_RESULT.material
                ) + Mdiffuse(RAY_RESULT.material)*0.2;

        color = light;

    }

    color = pow(color, vec3(1.0/0.6));
    out_color = vec4(color, 1.0);
}



