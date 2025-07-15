# RaymarchSandbox

#### Made with raylib (https://github.com/raysan5/raylib)  and imgui (https://github.com/ocornut/imgui)

 References for raymarching from iq: https://iquilezles.org/articles/
 
-----------------------------------

## Features
* Built-in functions for raymarching.
* Text editor with GLSL syntax highlight.
* Custom uniform inputs.  (texture input not implemented yet.)
* First person camera support.
-----------------------------------




## Building instructions.

```bash
git clone https://github.com/331uw13/RaymarchSandbox.git
cd RaymarchSandbox
git clone https://github.com/ocornut/imgui.git
make
./rmsb examples/intro.glsl
```

-----------------------------------



## Recent Updates

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


