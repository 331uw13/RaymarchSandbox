# RaymarchSandbox

#### Made with raylib (https://github.com/raysan5/raylib)  and imgui (https://github.com/ocornut/imgui)

## Features
* Built-in functions for raymarching.
* Text editor with GLSL syntax highlight.
* Custom uniform inputs.  (texture input not implemented yet.)
* First person camera support.
-----------------------------------

 References for raymarching from iq: https://iquilezles.org/articles/



### (screenshots are not updated)

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
## Recent Updates

* Thu Jul 3 2025
```
- Custom uniform inputs will now remember their values even when restarted.
- Reflections now work with fog correctly.
- Small improvements.
```

* Wed Jul 2 2025
```
- Moved to using compute shader for raymarching.
- Internal library is now read from `internal.glsl` file
- Added reflection option "MreflectN" for material.
```

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







