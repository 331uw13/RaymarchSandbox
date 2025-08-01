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
```
* `render_resolution` Options: FULL, HALF or CUSTOM

-----------------------------------

## Want to contribute?

If you have a new feature or even small improvement in mind feel free to open an "issue" or submit a pull request.

There are not strict rules about code style, but try to keep the code readable and use descriptive names for functions and variables.

-----------------------------------

![image](https://github.com/331uw13/RaymarchSandbox/blob/main/screenshots/raymarchsandbox_0.png?raw=true)

![image](https://github.com/331uw13/RaymarchSandbox/blob/main/screenshots/soft_shadows_and_ao.png?raw=true)

![image](https://github.com/331uw13/RaymarchSandbox/blob/main/screenshots/experiment_3.png?raw=true)

![image](https://github.com/331uw13/RaymarchSandbox/blob/main/screenshots/rmsb-intro.png?raw=true)


-----------------------------------


## Recent Updates

* Fri Aug  1 2025
```
- Raylib tracelog, OpenGL errors and other reasonable info are now written into "rmsb.log"
- Added Soft shadows for directional light.
- Small improvements.
```

* Wed Jul 30 2025
```
- Changes to uniform saving:
   - New format is "uniform_name"(RGBA/XYZ/SINGLE)[valueX, valueY, valueZ, valueW]
   - RGBA: Color.
   - XYZ: Position.
   - SINGLE: One floating point value
   - Before there was issues with saving new added uniforms from gui, that should also be fixed now.
- Added position input for shaders
```

* Tue Jul 29 2025
```
- Added file browser.
- Shaders can be opened with file browser now.
- Added config file using: https://github.com/benhoyt/inih
```

* Sun Jul 27 2025
```
- Added ambient occlusion.
- Added mandelbulb fractal example.
- Updated internal.glsl
- Small improvements.
```

* Mon Jul 21 2025
```
- Changes to fog.
- Fixed fog blending with translucent materials.
- Added soft shadows.
```

* Tue Jul 15 2025
```
- Changed how colors are handled for rays.
- Changed undo functionality.
- Updated internal.glsl
- Shader errors now report the correct row for the user
- Changed examples
```

