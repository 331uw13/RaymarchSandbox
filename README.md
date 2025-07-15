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
`internal.glsl` is the "library" for the Raymarch Sandbox.
It has all the functions written to allow user to create 3D scenes.
By default it contains useful functions for raymarching, coloring the materials, gradient noise functions and miscellaneous utilities.

 Reloading it is also supported at runtime.

-----------------------------------

![image](https://github.com/331uw13/RaymarchSandbox/blob/main/screenshots/rmsb-intro.png?raw=true)


-----------------------------------

## Recent Updates

* Tue Jul 15 2025
```
- Changed how colors are handled for rays.
- Changed undo functionality.
- Updated internal.glsl
- Shader errors now report the correct row for the user
- Changed examples
```

* Sun Jul 13 2025
```
- Started to implement undo for editor
```

* Thu Jul 10 2025
```
- Created more examples and intro.glsl
- Updated internal.glsl
- Fixed few bugs
```

* Tue Jul 8 2025
```
- Added translucent material property.
- Small improvements
```

