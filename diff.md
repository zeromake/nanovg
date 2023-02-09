## compare to inniyah/nanovg

- `nanovg.h` add `nvgBarc` method.
- `nanovg.h` add `nvgCreateImageAlpha` method.
- `nanovg_gl.h` add `glew_initialized` init glew.
- `nanovg_gl.h` add `NanoVG_GL_Functions_VTable` support multi platform switching.

## compare to awtk/3rd/nanovg

- `nanovg.h` add `NVGorientation`, `nvgBeginFrameEx`.
- `nanovg.h` add `nvgGetStateXfrom`, `nvgIntersectScissor_ex`, `nvgGetCurrScissor`, `nvgIntersectScissorForOtherRect` support more scissor.
- `nanovg.h` add NVGparams's `setLineCap`, `setLineJoin`, `clearCache`, `findTexture`, `setStateXfrom`, `globalSreenOrientation` support agg, agge.
- `nanovg.c` nvgEndFrame clear all images after j
- `nanovg_gl.h` support 