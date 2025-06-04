


Material map(vec3 p) {
    Material test = Material(0);

    p = REPEAT_INF(p, vec3(3, 8, 3));

    Mdistance(test) = SPHERE_SDF(p, 0.5);
    Mdiffuse(test) = vec3(0.2, 0.4, 0.2);


    return test;
}



void main() {
    vec3 color = vec3(0);

    vec3 ro = vec3(0, 0, -8.0);
    vec3 rd = RAYDIR();
  
    rd *= ROTATE_m3(vec2(time*0.2));

    RAYMARCH(ro, rd);
    if(RAY_RESULT.hit == 1) {
        color = COMPUTE_NORMAL(RAY_RESULT.pos);
    }

    out_color = vec4(color, 1.0);
}

