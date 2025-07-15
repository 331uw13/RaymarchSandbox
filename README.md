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


![image](https://github.com/331uw13/RaymarchSandbox/blob/main/screenshots/rmsb-intro.png?raw=true)


## Recent Updates

* Tue Jul 15 2025
```
- Changed how colors are handled for rays.
- Changed undo functionality.
- Updated internal.glsl
- Shader errors now report the correct row for the user
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

