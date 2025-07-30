/*
    This is simple example of what kind of things you can do.
    
    
    - Move this editor with holding right mouse button.
    
    - Camera control:
      you have to be in View_Mode (Left Control + X)

      then enable camera input (Left Control + C)
      You can then switch back to Edit_Mode with (Left Control + X)
    
    - (Left Control + E): Hide/Show Editor.
    - (Left Control + F): Hide/Show Settings & Functions.
    - (Left Control + A): Hide/Show Gui.
    
    - Added uniforms from "Uniform Input" tab
      are available in the code with the same name
      after reloading the shader.
 
    Report bugs to the project github page or dm me in discord: _331uw13
*/


vec3 light_pos = vec3(cos(time)*8.0, 1.0, sin(time)*8.0+16);

Material map(vec3 p) {


    // Offsets.
    p.z -= 15.0;
    p.y += 8;
    
    // GROUND
    float pattern = floor(sin(p.x)*2.0+sin(p.z)*2.0)+PI*2;
    Material ground = EmptyMaterial();
    Mdistance(ground) = BoxSDF(p, vec3(500.0, 0.2, 500.0));
    Mdiffuse(ground) = vec3(1.0, 1.0, 1.0) * pattern*0.02; 
    Mspecular(ground) = vec3(1.0);
        
    // REFLECTIVE SHAPE
    vec3 sphere1_pos = p + vec3(-1.0, -8.0, -2.0);
    Material sphere1 = EmptyMaterial();
    Mdistance(sphere1) = SphereSDF(sphere1_pos, 2.0);
    Mdiffuse(sphere1) = test_color.rgb;
    Mspecular(sphere1) = vec3(0.3);
    MreflectN(sphere1) = sin(time)*0.5+0.5;
    
    // NON-REFLECTIVE SPHERE
    vec3 sphere2_pos = p + vec3(3.0, -8.0, -2.0);
    Material sphere2 = EmptyMaterial();
    Mdistance(sphere2) = SphereSDF(sphere2_pos, 1.0);
    Mdiffuse(sphere2) = test_color.rgb;
    Mspecular(sphere2) = vec3(0.3);
    
    // TRANSPARENT BOX FRAME
    vec3 tbox_pos = p + vec3(3.0, -5.0, 2.0);
    tbox_pos *= RotateM3(vec2(time*0.2, time*0.5));
    Material tbox = EmptyMaterial();
    Mdistance(tbox) = BoxFrameSDF(tbox_pos, vec3(1.5), 0.2)-0.1;
    Mdiffuse(tbox) = transparent_color.rgb;
    Mopaque(tbox) = 0;
    
    // TRANSPARENT TORUS
    vec3 torus_pos = p + vec3(-3.0, -5.0, 2.0);
    torus_pos *= RotateM3(vec2(time*0.2, time*0.5));
    Material torus = EmptyMaterial();
    Mdistance(torus) = TorusSDF(torus_pos, vec2(1.5, 0.5))-0.1;
    Mdiffuse(torus) = Palette(length(torus_pos*0.5), RAINBOW_PALETTE);
    Mopaque(torus) = 0;
    
    // GRID
    p.y += 6.5;
    Material grid = EmptyMaterial();
    vec3 spacing = vec3(13.0, 16.0, 13.0);
    vec3 q = RepeatINF(p+(spacing/2.0), spacing);
    Mdistance(grid) = BoxFrameSDF(q, vec3(2.0), 0.5);
    Mdiffuse(grid) = vec3(0.2);
    
    
    Material box = EmptyMaterial();
    float frame_size = 0.2;
    vec3 box_pos = p + vec3(3, -18, 0);
    box_pos *= RotateM3(vec2(-time*0.2, time));
    Mdistance(box) = BoxFrameSDF(box_pos, vec3(1.5), frame_size);
    Mdiffuse(box) = vec3(0.5, 0.5, 0.5);
    MreflectN(box) = 0.9;
    
    Material innerbox = EmptyMaterial();
    Mdistance(innerbox) = BoxSDF(box_pos, vec3(1.5-frame_size*2));
    Mdiffuse(innerbox) = Palette(length(box_pos)+time, RAINBOW_PALETTE );
    Mopaque(innerbox) = 0;
    
    // MaterialMin can be used to get the closest material
    // to current ray position.
    Material result = ground;
    result = MaterialMin(result, tbox);
    result = MaterialMin(result, torus);
    result = MaterialMin(result, sphere1);
    result = MaterialMin(result, sphere2);
    result = MaterialMin(result, grid);
    result = MaterialMin(result, innerbox);
    result = MaterialMin(result, box);
    
    return result;
}


vec3 raycolor_translucent() {
    float density = 1.0 / (Ray.vm_len * 2.0);
    return Mdiffuse(Ray.mat) * density;
}


vec3 raycolor() {
    vec3 color = vec3(0, 0, 0);
    
    if(Ray.hit == 0) {
        return fog_color.rgb + color;
    }
    
    vec3 normal = ComputeNormal(Ray.pos);
    float light_strength = 1.5;
    vec3 light = LightDirectional(
        CameraInputPosition,
        vec3(8.0, 10.0, -3.0),
        vec3(1.0, 0.9, 0.8),
        normal,
        Ray.mat
    ) * light_strength;
    vec3 ambient = Mdiffuse(Ray.mat) * 0.02;

    color = light + ambient;

    return color;
}



void entry() {

    FOG_COLOR = fog_color.rgb;
    FOG_DENSITY = 1.0;
    FOG_EXPONENT = 2.0;
    
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

//ADD COLOR test_color (0.948, 0.004, 0.277, 1.000);
//ADD COLOR light_color (0.560, 1.000, 0.108, 0.882);
//ADD COLOR fog_color (0.944, 0.320, 0.254, 0.000);
//ADD COLOR transparent_color (0.145, 0.723, 0.660, 1.000);

@_UNIFORM_METADATA_END

