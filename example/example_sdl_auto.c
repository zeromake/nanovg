#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include "nanovg.h"
#define NANOVG_IMPLEMENTATION
#include "nanovg_wrapper.h"

#ifdef ANDROID
#include <android/log.h>
#define LOG_TAG "com.zeromake.example"
#define printf(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#endif

// #ifdef USE_FPS
// #include "perf.h"
// #include "perf.c"
// #endif

double GetElapsedTime(Uint64 start, double frequency)
{
    return (double)(SDL_GetPerformanceCounter() - start) / frequency;
}

int main(int argc, char **argv)
{
    int initWindowflags = nvgInitSDLDirver();
    int windowflags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | initWindowflags;
    windowflags |= SDL_WINDOW_RESIZABLE;
    SDL_Window *window = SDL_CreateWindow(
        "NanoVG Example",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        800,
        600,
        windowflags);
    int createFlags = NVG_ANTIALIAS | NVG_STENCIL_STROKES;
    NVGcontext *vg = nvgCreate(createFlags, window);
    if (vg == NULL)
    {
        printf("ERROR: NanoVG init failed");
        return EXIT_FAILURE;
    }
    NVGrendererInfo info = nvgGetRendererInfo(vg);
    printf("render system:\n");
    printf("  renderer:     %s\n", info.rendererName);
    printf("  device:       %s\n", info.deviceName);
    printf("  vendor:       %s\n", info.vendorName);
    printf("  shader:       %s\n", info.shaderName);
    printf("\n");

    int winWidth = 0, winHeight = 0;
    SDL_GetWindowSize(window, &winWidth, &winHeight);
    int fbWidth = winWidth;
    int fbHeight = winHeight;
    SDL_GetWindowSizeInPixels(window, &fbWidth, &fbHeight);
    float fbRatio = nvgDevicePixelRatio(vg);
    nvgResetFrameBuffer(vg, fbWidth, fbHeight);

    int quit = 0;
    SDL_Event event;
    NVGcolor bgColor = nvgRGBA(0xef, 0xe6, 0xc7, 255);
    while (!quit)
    {
        SDL_PollEvent(&event);

        switch (event.type)
        {
        case SDL_QUIT:
            quit = 1;
            break;
        case SDL_WINDOWEVENT:
            switch (event.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    // 窗口大小变化完成
                    SDL_GetWindowSize(window, &winWidth, &winHeight);
                    SDL_GetWindowSizeInPixels(window, &fbWidth, &fbHeight);
                    fbRatio = nvgDevicePixelRatio(vg);
                    nvgResetFrameBuffer(vg, fbWidth, fbHeight);
                    break;
            }
            break;
        }

        nvgClearWithColor(vg, bgColor);
        nvgBeginFrame(vg, fbWidth, fbHeight, fbRatio);

        nvgBeginPath(vg);
        nvgRect(vg, 50, 50, 200, 200);
        nvgFillColor(vg, nvgRGBA(192, 192, 192, 255));
        nvgFill(vg);

        nvgEndFrame(vg);
        nvgPresent(vg);
    }
    nvgDelete(vg);
    SDL_DestroyWindow(window);
    return 0;
}
