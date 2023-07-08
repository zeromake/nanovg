
#ifndef NANOVG_WRAPPER_H
#define NANOVG_WRAPPER_H

#include "nanovg.h"

NVGcontext* nvgCreate(int flags, void* params);
void nvgDelete(NVGcontext* ctx);
void nvgClearWithColor(NVGcontext* ctx, NVGcolor color);
void nvgClearRectWithColor(NVGcontext* ctx, NVGcolor color, int* rect);
void nvgResetFrameBuffer(NVGcontext* ctx, int width, int height);
float nvgDevicePixelRatio(NVGcontext* ctx);
void nvgPresent(NVGcontext* ctx);

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
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#ifdef _WIN32
#include <windows.h>
WINUSERAPI UINT WINAPI GetDpiForWindow(HWND hwnd);
#endif

void nvgClearWithColor(NVGcontext* ctx, NVGcolor color) {
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
}

void nvgClearRectWithColor(NVGcontext* ctx, NVGcolor color, int* rect) {
    glEnable(GL_SCISSOR_TEST);
    glScissor((GLint)rect[0], (GLint)rect[1], (GLsizei)rect[2], (GLsizei)rect[3]);
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
}

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
    nvgSetUserPtr(ctx, NULL);
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

float nvgDevicePixelRatio(NVGcontext* ctx) {
    SDL_SysWMinfo info;
    SDL_GetVersion(&info.version);
    SDL_Window* window = (SDL_Window*)nvgGetUserPtr(ctx);
    SDL_GetWindowWMInfo(window, &info);
#ifdef _WIN32
    return (float)GetDpiForWindow(info.info.win.window) / 96.0f;
#elif defined(__APPLE__)
#ifdef NANOVG_USE_GL
    return 2.0f;
#else
    NSWindow* win = info.info.cocoa.window;
    return [win backingScaleFactor];
#endif
#else
    return 1.0f;
#endif
}

void nvgPresent(NVGcontext* ctx) {
    SDL_Window* window = (SDL_Window*)nvgGetUserPtr(ctx);
    SDL_GL_SwapWindow(window);
}

#elif defined(NANOVG_USE_D3D11)
#define NANOVG_D3D11_IMPLEMENTATION
#include "nanovg_d3d11.h"
#include "d3d11_helper.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

NVGcontext* nvgCreate(int flags, void* params) {
    SDL_SysWMinfo info;
    SDL_GetVersion(&info.version);
    SDL_Window* window = (SDL_Window*)params;
    SDL_GetWindowWMInfo(window, &info);
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window, &w, &h);
    D3D11Context* ctx = CreateD3D11Context((void*)info.info.win.window, w, h);
    ID3D11Device *device = (ID3D11Device*)GetD3D11Device(ctx);
    NVGcontext* vg = nvgCreateD3D11(device, flags);
    nvgSetUserPtr(vg, (void*)ctx);
    return vg;
}

void nvgDelete(NVGcontext* ctx) {
    void* d3d11Ctx = nvgGetUserPtr(ctx);
    nvgSetUserPtr(ctx, NULL);
    nvgDeleteD3D11(ctx);
    DestroyD3D11Context((D3D11Context*)d3d11Ctx);
    d3d11Ctx = NULL;
}

void nvgResetFrameBuffer(NVGcontext* ctx, int width, int height) {
    ResizeD3D11Drawable((D3D11Context*)nvgGetUserPtr(ctx), width, height);
}

float nvgDevicePixelRatio(NVGcontext* ctx) {
    return GetD3D11ScaleFactor((D3D11Context*)nvgGetUserPtr(ctx));
}

void nvgClearWithColor(NVGcontext* ctx, NVGcolor color) {
    ClearD3D11WithColor((D3D11Context*)nvgGetUserPtr(ctx), color.rgba);
}

void nvgPresent(NVGcontext* ctx) {
    D3D11Present((D3D11Context*)nvgGetUserPtr(ctx));
}

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
    void* metalCtx = nvgGetUserPtr(ctx);
    nvgSetUserPtr(ctx, NULL);
    nvgDeleteMTL(ctx);
    DestroyMetalContext((MetalContext*)metalCtx);
    metalCtx = NULL;
}

void nvgResetFrameBuffer(NVGcontext* ctx, int width, int height) {
    ResizeMetalDrawable((MetalContext*)nvgGetUserPtr(ctx), width, height);
}

float nvgDevicePixelRatio(NVGcontext* ctx) {
    return GetMetalScaleFactor((MetalContext*)nvgGetUserPtr(ctx));
}
void nvgPresent(NVGcontext* ctx) {}
#else
#error "you need define NANOVG_USE_GL|NANOVG_USE_D3D11|NANOVG_USE_METAL"
#endif
#endif

#endif
