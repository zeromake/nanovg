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
#include "perf.h"
#include "demo.h"
#include "perf.c"
// #endif

double GetElapsedTime(Uint64 start, double frequency)
{
    return (double)(SDL_GetPerformanceCounter() - start) / frequency;
}

#ifdef main
#undef main
#endif

int main(int argc, char **argv)
{
    int flags = SDL_INIT_EVERYTHING & ~(SDL_INIT_TIMER | SDL_INIT_HAPTIC);
    if (SDL_Init(flags) < 0) {
        printf("ERROR: SDL_Init failed: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    Uint64 start = SDL_GetPerformanceCounter();
    double frequency = (double)SDL_GetPerformanceFrequency();
    int initWindowflags = nvgInitSDLDirver();
    int windowflags = SDL_WINDOW_HIDDEN | SDL_WINDOW_ALLOW_HIGHDPI | initWindowflags;
    windowflags |= SDL_WINDOW_RESIZABLE;
    SDL_Window *window = SDL_CreateWindow(
        "NanoVG Example",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        1000,
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

#ifndef __APPLE__
    if (fbRatio > 1) {
        winWidth *= fbRatio;
        winHeight *= fbRatio;
        SDL_SetWindowSize(window, winWidth, winHeight);
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }
#endif
    SDL_ShowWindow(window);
    int quit = 0;
    SDL_Event event;
    NVGcolor bgColor = nvgRGBA(0x76, 0x76, 0x82, 255);
    double mx, my;
    int premult = 0;
    int blowup = 0;
    double t = 0;
    double dt = 0;
    float frameDevicePixelRatio = 1.0;
#ifndef NANOVG_USE_D3D11
    frameDevicePixelRatio = fbRatio;
#endif
	DemoData data;
	GPUtimer gpuTimer;
	PerfGraph fps, cpuGraph, gpuGraph;
	double prevt = 0, cpuTime = 0;
    prevt = GetElapsedTime(start, frequency);
    initGraph(&fps, GRAPH_RENDER_FPS, "Frame Time");
	initGraph(&cpuGraph, GRAPH_RENDER_MS, "CPU Time");
	initGraph(&gpuGraph, GRAPH_RENDER_MS, "GPU Time");

    initGPUTimer(&gpuTimer);
    if (loadDemoData(vg, &data) == -1)
		return -1;
    while (!quit)
    {
        t = GetElapsedTime(start, frequency);
		dt = t - prevt;
		prevt = t;
        startGPUTimer(&gpuTimer);
        SDL_PollEvent(&event);
        float gpuTimes[3];

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
                    #ifndef NANOVG_USE_D3D11
                        frameDevicePixelRatio = fbRatio;
                    #endif
                    nvgResetFrameBuffer(vg, fbWidth, fbHeight);
                    break;
            }
            break;
        case SDL_MOUSEMOTION:
            mx = event.motion.x;
            my = event.motion.y;
#ifndef __APPLE__
            mx /= fbRatio;
            mx /= fbRatio;
#endif
        }

        nvgClearWithColor(vg, bgColor);
        nvgBeginFrame(vg, fbWidth, fbHeight, frameDevicePixelRatio);

        int w = winWidth;
        int h = winHeight;
#ifndef __APPLE__
        w /= fbRatio;
        h /= fbRatio;
    	nvgScale(vg, fbRatio, fbRatio);
#endif

        renderDemo(vg, mx, my, w, h, t, blowup, &data);
        renderGraph(vg, 5,5, &fps);
		renderGraph(vg, 5+200+5,5, &cpuGraph);
		if (gpuTimer.supported)
			renderGraph(vg, 5+200+5+200+5,5, &gpuGraph);

        nvgEndFrame(vg);
        cpuTime = GetElapsedTime(start, frequency) - t;

		updateGraph(&fps, dt);
		updateGraph(&cpuGraph, cpuTime);

		// We may get multiple results.
		int n = stopGPUTimer(&gpuTimer, gpuTimes, 3);
		for (int i = 0; i < n; i++)
			updateGraph(&gpuGraph, gpuTimes[i]);
        nvgPresent(vg, 1);
    }
	freeDemoData(vg, &data);
    nvgDelete(vg);
    SDL_DestroyWindow(window);
    return 0;
}
