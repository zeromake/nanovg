
#ifndef NANOVG_WRAPPER_H
#define NANOVG_WRAPPER_H

#include "nanovg.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct NVGScreenshotTexture {
    int image;
    int height;
    int width;
    unsigned char *pixel;
} NVGScreenshotTexture;

NVGcontext* nvgCreate(int flags, void* params);
void nvgDelete(NVGcontext* ctx);
void nvgClearWithColor(NVGcontext* ctx, NVGcolor color);
void nvgClearRectWithColor(NVGcontext* ctx, NVGcolor color, int* rect);
void nvgResetFrameBuffer(NVGcontext* ctx, int width, int height);
float nvgDevicePixelRatio(NVGcontext* ctx);
void nvgPresent(NVGcontext* ctx);
void* nvgCreateFramebuffer(NVGcontext* ctx, int w, int h, int flags);
void nvgBindFramebuffer(NVGcontext* ctx, void* fb);
void nvgDeleteFramebuffer(NVGcontext* ctx, void* fb);
NVGScreenshotTexture* nvgScreenshotTexture(NVGcontext* ctx, int *rect);
bool nvgScreenshotSave(NVGcontext* ctx, NVGScreenshotTexture* tex, char *out);

NVGpaint nvgFramebufferPattern(
    NVGcontext* ctx,
    float cx,
    float cy,
    float w,
    float h,
    float angle,
    void* fb,
    float alpha
);

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
#include "nanovg_gl_utils.h"
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
    char *apiName = "OpenGL";
#if defined(NANOVG_USE_GL2)
    vg = nvgCreateGL2(flags);
    const char *shadingLanguageName = "GLSL2";
#elif defined(NANOVG_USE_GL3)
    vg = nvgCreateGL3(flags);
    const char *shadingLanguageName = "GLSL3";
#elif defined(NANOVG_USE_GLES2)
    apiName = "OpenGL ES";
    const char shadingLanguageName = "ESSL2";
    vg = nvgCreateGLES2(flags);
#elif defined(NANOVG_USE_GLES3)
    apiName = "OpenGL ES";
    const char shadingLanguageName = "ESSL3";
    vg = nvgCreateGLES3(flags);
#endif
    nvgSetUserPtr(vg, params);
    char nameBuffer[256] = {0};
    sprintf(nameBuffer, "%s %s", apiName, (char *)glGetString(GL_VERSION));
    NVGrendererInfo renderInfo = {0};
    strcat(renderInfo.rendererName, nameBuffer);
    sprintf(nameBuffer, "%s %s", shadingLanguageName, (char *)glGetString(GL_SHADING_LANGUAGE_VERSION));
    strcat(renderInfo.shaderName, nameBuffer);
    strcat(renderInfo.deviceName, glGetString(GL_RENDERER));
    strcat(renderInfo.vendorName, glGetString(GL_VENDOR));
    nvgSetRendererInfo(vg, renderInfo);
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

void* nvgCreateFramebuffer(NVGcontext* ctx, int w, int h, int flags) {
    return nvgluCreateFramebuffer(ctx, w, h, flags);
}
void nvgBindFramebuffer(NVGcontext* ctx, void* fb) {
    nvgluBindFramebuffer((NVGLUframebuffer*)fb);
}
void nvgDeleteFramebuffer(NVGcontext* ctx, void* fb) {
    nvgluDeleteFramebuffer((NVGLUframebuffer*)fb);
}
NVGpaint nvgFramebufferPattern(
    NVGcontext* ctx,
    float cx,
    float cy,
    float w,
    float h,
    float angle,
    void* fb,
    float alpha
) {
    return nvgImagePattern(
        ctx,
        cx,
        cy,
        w,
        h,
        angle,
        ((NVGLUframebuffer*)fb)->image,
        alpha
    );
}


static void flipHorizontal(unsigned char* image, int w, int h)
{
	int size = h / 2;
    int stride = w * 4;
    unsigned char *swap = (unsigned char *)malloc(stride);
	for (int i = 0; i < size; i++) {
		unsigned char* ri = &image[i * stride];
		unsigned char* rj = &image[(h - i - 1) * stride];
        memcpy(swap, ri, stride);
        memcpy(ri, rj, stride);
        memcpy(rj, swap, stride);
	}
    free(swap);
}

NVGScreenshotTexture* nvgScreenshotTexture(NVGcontext* ctx, int* rect) {
    int x = rect[0];
    int y = rect[1];
    int w = rect[2];
    int h = rect[3];
    unsigned char* pixel = (unsigned char*)malloc(w*h*4);
    glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    flipHorizontal(pixel, w, h);
    int image = nvgCreateImageRGBA(ctx, w, h, 0, pixel);
    NVGScreenshotTexture* tex = (NVGScreenshotTexture*)malloc(sizeof(NVGScreenshotTexture));
    tex->image = image;
    tex->pixel = pixel;
    tex->width = w;
    tex->height = h;
    return tex;
}

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

bool nvgScreenshotSave(NVGcontext* ctx, NVGScreenshotTexture* tex, char *out) {
    int stride = tex->width * 4;
    int size = tex->width*tex->height*4;
 	stbi_write_png(out, tex->width, tex->height, 4, tex->pixel, stride);
    return true;
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
    NVGrendererInfo renderInfo = D3D11GetRenderInfo(ctx);
    ID3D11Device *device = (ID3D11Device*)GetD3D11Device(ctx);
    NVGcontext* vg = nvgCreateD3D11(device, flags);
    nvgSetRendererInfo(vg, renderInfo);
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
    D3D11Present((D3D11Context*)nvgGetUserPtr(ctx), 1);
}

NVGScreenshotTexture* nvgScreenshotTexture(NVGcontext* ctx, int *rect) {
    D3D11Context* c = (D3D11Context*)nvgGetUserPtr(ctx);
    ID3D11Texture2D* t = D3D11GetSwapChainTexture(c);
    int image = nvgTexture2ImageD3D11(ctx, t, rect);
    D3D_API_RELEASE(t);
    NVGScreenshotTexture* tex = (NVGScreenshotTexture*)malloc(sizeof(NVGScreenshotTexture));
    tex->image = image;
    tex->width = rect[2];
    tex->height = rect[3];
    tex->pixel = NULL;
    return tex;
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
    NVGrendererInfo renderInfo = MetalGetRenderInfo(ctx);
    nvgSetRendererInfo(vg, renderInfo);
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

void* nvgCreateFramebuffer(NVGcontext* ctx, int w, int h, int flags) {
    return mnvgCreateFramebuffer(ctx, w, h, flags);
}
void nvgBindFramebuffer(NVGcontext* ctx, void* fb) {
    mnvgBindFramebuffer(ctx, (MNVGframebuffer*)fb);
}
void nvgDeleteFramebuffer(NVGcontext* ctx, void* fb) {
    mnvgDeleteFramebuffer((MNVGframebuffer*)fb);
}
NVGpaint nvgFramebufferPattern(
    NVGcontext* ctx,
    float cx,
    float cy,
    float w,
    float h,
    float angle,
    void* fb,
    float alpha
) {
    return nvgImagePattern(
        ctx,
        cx,
        cy,
        w,
        h,
        angle,
        ((MNVGframebuffer*)fb)->image,
        alpha
    );
}

NVGScreenshotTexture* nvgScreenshotTexture(NVGcontext* ctx, int* rect) {
    int image = GetMetalScreenshotTexture(ctx, rect[2], rect[3]);
    NVGScreenshotTexture* tex = (NVGScreenshotTexture*)malloc(sizeof(NVGScreenshotTexture));
    tex->image = image;
    tex->width = rect[2];
    tex->height = rect[3];
    tex->pixel = NULL;
    return tex;
}
#else
#error "you need define NANOVG_USE_GL|NANOVG_USE_D3D11|NANOVG_USE_METAL"
#endif
#endif

#endif
