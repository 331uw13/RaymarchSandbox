# RaymarchSandbox

#### Made with raylib (https://github.com/raysan5/raylib)  and imgui (https://github.com/ocornut/imgui)

 References for raymarching from iq: https://iquilezles.org/articles/
 
-----------------------------------

## Features
* Built-in functions for raymarching.
* Text editor with GLSL syntax highlight.
* Custom uniform inputs.  (texture input not implemented yet.)
* First person camera support.
* Reflective materials.
* Translucent materials.
* Ambient occlusion.
* Soft shadows.

-----------------------------------


## Building instructions.

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


![image](https://github.com/331uw13/RaymarchSandbox/blob/main/screenshots/raymarchsandbox_0.png?raw=true)

![image](https://github.com/331uw13/RaymarchSandbox/blob/main/screenshots/soft_shadows_and_ao.png?raw=true)

![image](https://github.com/331uw13/RaymarchSandbox/blob/main/screenshots/experiment_3.png?raw=true)

![image](https://github.com/331uw13/RaymarchSandbox/blob/main/screenshots/rmsb-intro.png?raw=true)


-----------------------------------

## Recent Updates

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

