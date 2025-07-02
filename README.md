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
![image](https://github.com/331uw13/RaymarchSandbox/blob/main/screenshots/raymarch-sandbox-3.png?raw=true)

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







