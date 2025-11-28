# RaymarchSandbox

Made with Raylib (https://github.com/raysan5/raylib) and ImGui (https://github.com/ocornut/imgui)

Also check out Inigo Quilez's articles about raymarching: https://iquilezles.org/articles/
 
-----------------------------------

## Features
* "Built-in" functions for raymarching.
* Text editor with GLSL syntax highlight.
* Custom uniform inputs.  (texture input not implemented yet.)
* First person camera support.
* Reflective materials.
* Translucent materials.
* Ambient occlusion.
* Soft shadows.

-----------------------------------


## Building instructions.

> Currently **only** GNU/Linux is supported.

```bash
git clone https://github.com/331uw13/RaymarchSandbox.git
cd RaymarchSandbox
git clone https://github.com/ocornut/imgui.git
make -j4
./rmsb examples/intro.glsl
```

(`examples/intro.glsl` has instructions for getting started)

-----------------------------------

## About internal.glsl
> [!NOTE]
> Work in progress. Some functions may change without warning.

`internal.glsl` is the "library" for the Raymarch Sandbox.
It has all the functions written to allow user to create 3D scenes.
By default it contains useful functions for raymarching, coloring the materials, gradient noise functions and miscellaneous utilities.

 Reloading it is also supported at runtime.

-----------------------------------

## Configuration file (rmsb.ini)
```ini
[render_settings]
fps_limit = 300
fov = 60.0
hit_distance = 0.001
mx_ray_length = 300.0
ao_step = 0.01
ao_samples = 32
ao_falloff = 3.0
translucent_step = 0.1

render_resolution = FULL
custom_render_resolution_X = 0
custom_render_resolution_Y = 0

[font_settings]
imgui_font = ./fonts/AdwaitaSans-Regular.ttf
editor_font = ./fonts/Px437_IBM_Model3x_Alt4.ttf
```
* `render_resolution` Options: FULL, HALF or CUSTOM

-----------------------------------

## Want to contribute?

If you have a new feature or even small improvement in mind feel free to open an "issue" or submit a pull request.

There are not strict rules about code style, but try to keep the code readable and use descriptive names for functions and variables.

-----------------------------------

![image](https://github.com/331uw13/RaymarchSandbox/blob/main/screenshots/raymarch-sandbox_2.png?raw=true)

![image](https://github.com/331uw13/RaymarchSandbox/blob/main/screenshots/raymarch-sandbox_1.png?raw=true)

![image](https://github.com/331uw13/RaymarchSandbox/blob/main/screenshots/raymarch-sandbox_0.png?raw=true)


