
#ifndef NANOVG_WRAPPER_H
#define NANOVG_WRAPPER_H

#include "nanovg.h"

NVGcontext* nvgCreate(int flags, void* params);
void nvgDelete(NVGcontext* ctx);
void nvgClearWithColor(NVGcontext* ctx, NVGcolor color);
void nvgResetFrameBuffer(NVGcontext* ctx, int width, int height);

#ifdef NANOVG_IMPLEMENTATION
#if defined(NANOVG_USE_GL)
#if defined(NANOVG_USE_GL2)
#define NANOVG_GL2_IMPLEMENTATION
#elif defined(NANOVG_USE_GL3)
#define NANOVG_GL3_IMPLEMENTATION
#elif defined(NANOVG_USE_GLES2)
#define NANOVG_GLES2_IMPLEMENTATION
#elif defined(NANOVG_USE_GLES3)
#define NANOVG_GLES3_IMPLEMENTATION
#endif
#include "nanovg_gl.h"

NVGcontext* nvgCreate(int flags, void* params) {
    NVGcontext *vg = NULL;
#if defined(NANOVG_USE_GL2)
    vg = nvgCreateGL2(flags);
#elif defined(NANOVG_USE_GL3)
    vg = nvgCreateGL3(flags);
#elif defined(NANOVG_USE_GLES2)
    vg = nvgCreateGLES2(flags);
#elif defined(NANOVG_USE_GLES3)
    vg = nvgCreateGLES3(flags);
#endif
    nvgSetUserPtr(vg, params);
    return vg;
}

void nvgDelete(NVGcontext* ctx) {
#if defined(NANOVG_USE_GL2)
    nvgDeleteGL2(ctx);
#elif defined(NANOVG_USE_GL3)
    nvgDeleteGL3(ctx);
#elif defined(NANOVG_USE_GLES2)
    nvgDeleteGLES2(ctx);
#elif defined(NANOVG_USE_GLES3)
    nvgDeleteGLES3(ctx);
#endif
}

void nvgResetFrameBuffer(NVGcontext* ctx, int width, int height) {
    glViewport(0, 0, width, height);
}

#elif defined(NANOVG_USE_D3D11)
#define NANOVG_D3D11_IMPLEMENTATION
#include "nanovg_d3d11.h"
#elif defined(NANOVG_USE_METAL)
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include "nanovg_mtl.h"
#include "metal_helper.h"

NVGcontext* nvgCreate(int flags, void* params) {
    SDL_SysWMinfo info;
    SDL_GetVersion(&info.version);
    SDL_GetWindowWMInfo((SDL_Window*)params, &info);
    MetalContext* ctx = CreateMetalContext((void*)info.info.cocoa.window);
    void* metalLayer = GetMetalLayer(ctx);
    NVGcontext* vg = nvgCreateMTL(metalLayer, flags);
    nvgSetUserPtr(vg, (void*)ctx);
    return vg;
}

void nvgDelete(NVGcontext* ctx) {
    nvgDeleteMTL(ctx);
    void* metalCtx = nvgGetUserPtr(ctx);
    DestroyMetalContext((MetalContext*)metalCtx);
    metalCtx = NULL;
    nvgSetUserPtr(ctx, NULL);
}

void nvgResetFrameBuffer(NVGcontext* ctx, int width, int height) {
    ResizeMetalDrawable((MetalContext*)nvgGetUserPtr(ctx), width, height);
}
#else
#error "you need define NANOVG_USE_GL|NANOVG_USE_D3D11|NANOVG_USE_METAL"
#endif
#endif

#endif
