


Material map(vec3 p) {
    Material test = Material(0);

    Mdistance(test) = SphereSDF(p, 0.5);
    Mdiffuse(test) = vec3(test_color);
    Mshine(test) = 3.0;
    Mspecular(test) = vec3(test_value*20.0);

    return test;
}



void main() {
    vec3 color = vec3(0);
    vec3 ro = vec3(0, 0, -8.0);
    vec3 rd = RayDir(); 


    Camera.pos = ro;
    Raymarch(ro, rd);

    if(Ray.hit == 1) {
        vec3 normal = ComputeNormal(Ray.pos);
        vec3 light = ComputeLight(
                vec3(cos(time)*8.0, -10.0, sin(time)*4.0),
                vec3(1.0, 1.0, 1.0),
                normal,
                Ray.pos,
                Ray.material
                ) + Mdiffuse(Ray.material)*0.2;

        color = light;

    }

    color = pow(color, vec3(1.0/0.6));

    out_color = vec4(color, 1.0);
}



// This will add custom uniforms automatically when the shader is compiled the first time.
// They can be then modified at run time from "Input" tab. Or add new ones from the gui.
@startup_cmd

ADD COLOR test_color;
ADD VALUE test_value;

@end

