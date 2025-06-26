# RaymarchSandbox

#### Made with raylib (https://github.com/raysan5/raylib)  and imgui (https://github.com/ocornut/imgui)

## Features
* Built-in functions for raymarching.
* Text editor with GLSL syntax highlight.
* Custom uniform inputs.  (texture input not implemented yet.)
* First person camera support.
-----------------------------------

 References for raymarching from iq: https://iquilezles.org/articles/


![image](https://github.com/331uw13/RaymarchSandbox/blob/main/screenshots/raymarch-sandbox-2.png?raw=true)
(screenshot not updated)

-------------------------------------------------


## Building instructions.

```bash
git clone https://github.com/331uw13/RaymarchSandbox.git
cd RaymarchSandbox
git clone https://github.com/ocornut/imgui.git
make
./rmsb example.glsl
```
-------------------------------------------------
## The example shader.
```glsl
Material map(vec3 p) {
    Material test = Material(0);

    p.y += 6.0;
    p = RepeatINF(p, vec3(12.0));

    Mdistance(test) = SphereSDF(p, 2.0);
    Mdiffuse(test) = vec3(test_color); // Diffuse color.
    Mspecular(test) = vec3(0.48, 0.44, 0.42); // Specular color.
    Mshine(test) = 0.25;

    return test;
}

void main() {
    vec3 fog_color = vec3(0.96, 0.75, 0.58);
    vec3 color = fog_color;
    vec3 ro = vec3(0, 0, 0);
    vec3 rd = RayDir(); // Get initial ray direction.

    Camera.pos = ro;
    Raymarch(ro, rd);

    if(Ray.hit == 1) {
        vec3 normal = ComputeNormal(Ray.pos);
        vec3 light = ComputeLight(
                vec3(50, -50.0, 0), // Light position.
                ColorRGB(250, 240, 230), // Light color.
                normal, // Surface normal.
                Ray.pos,
                Ray.material
                ) + Mdiffuse(Ray.material)*0.2; // Ambient.

        color = light;
    }

    // Gamma correction and fog.
    color = pow(color, vec3(1.0/0.7));
    color = ApplyFog(color, Ray.length*0.0025, fog_color);

    out_color = vec4(color, 1.0);
}

// This will add custom uniforms automatically when the shader is compiled the first time.
// They can be then modified at run time from "Input" tab. Or add new ones from the gui.
@startup_cmd

ADD COLOR test_color;
ADD VALUE test_value;

@end

```

-------------------------------------------------
## Recent Updates

* Thu Jun 26 2025
```
- Added copy and cut of selected text functionality to GLSL editor.
- Fixed editor focus issue
```

* Mon Jun 23 2025
```
- "Quit" button will now ask to save the shader before quitting if necessary
- Added noise functions to internal lib.
- Small improvements
```

* Sun Jun 22 2025
```
- Added more functions to internal lib.
- Bug fixes.
```

* Sat Jun 21 2025
```
- Updated GLSL editor to remove selected text and keep preferred column for cursor.
- Other small updates and bug fixes for the editor.
- Added auto reload option and delay for it.
- Added fallback option. User can choose to fallback to previous working shader or not.
```







