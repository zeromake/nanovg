#include <stdio.h>
#include <stdlib.h>


#ifdef NANOVG_GLEW
	#include <GL/glew.h>
#else
	#include <glad/glad.h>
#endif
#include <SDL.h>
#include <SDL_video.h>
#include "demo.h"
#include "perf.h"
// #include <SDL_opengl.h>
// #include <SDL_image.h>
// #include <SDL_ttf.h>

#include <nanovg.h>
#include <nanovg_gl.h>
#include <nanovg_gl_utils.h>


int blowup = 0;
int screenshot = 0;
int premult = 0;

int main(int argc, char **argv) {

	DemoData data;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		return EXIT_FAILURE;
	}

	SDL_GLContext context;
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,  SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,          1);
	// SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,    1);
	// SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,    8);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	SDL_Window *window = SDL_CreateWindow(
		"SDL2/OpenGL/NanoVG",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		1024,
		800,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
	);


	context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, context);

#ifdef NANOVG_GLEW
	glewExperimental = 1;
	glewInit();
#else
	gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
#endif
	SDL_GL_SetSwapInterval(1);

	NVGcontext *vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
	if (vg == NULL) {
		return EXIT_FAILURE;
	}
	if (loadDemoData(vg, &data) == -1)
		return -1;

	int winWidth, winHeight;
	int fbWidth, fbHeight;
	float pxRatio;

	SDL_GetWindowSize(window, &winWidth, &winHeight);
	SDL_GL_GetDrawableSize(window, &fbWidth, &fbHeight);

	pxRatio =  (float)fbWidth / (float)winWidth;

	SDL_Event event;

	int running = 1;

	double prevt = 0, cpuTime = 0;
	PerfGraph fps, cpuGraph;
	initGraph(&fps, GRAPH_RENDER_FPS, "Frame Time");
	initGraph(&cpuGraph, GRAPH_RENDER_MS, "CPU Time");
	prevt = SDL_GetTicks() / 1e3;

	while (running) {
		double mx, my, t, dt;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = 0;
			}
			if (event.type == SDL_MOUSEMOTION) {
				mx = event.motion.x;
				my = event.motion.y;
			}
			if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == SDL_BUTTON_RIGHT) {
				}
				if (event.button.button == SDL_BUTTON_MIDDLE) {
				}
				if (event.button.button == SDL_BUTTON_LEFT) {
					break;
				}
			}
			if (event.type == SDL_KEYDOWN) {
				switch(event.key.keysym.sym ) {
					case SDLK_ESCAPE:
						running = 0;
						break;
					case SDLK_p:
						premult = !premult;
						break;
					case SDLK_SPACE:
						blowup = !blowup;
						break;
					default:
						break;
				}
			}
		}
		t = SDL_GetTicks() / 1e3;

		dt = t - prevt;
		prevt = t;
		// nvgBeginFrame(vg, winWidth, winHeight, pxRatio);
		// 	nvgBeginPath(vg);
		// 	nvgRect(vg, 300, 100, 120, 30);
		// 	nvgFillColor(vg, nvgRGBA(255, 192, 0, 255));
		// 	nvgFill(vg);
		// nvgEndFrame(vg);
		// Update and render
		glViewport(0, 0, fbWidth, fbHeight);
		NVGcolor clearColor;
		if (premult)
			clearColor = nvgRGBAf(0.0f, 0.0f, 0.0f, 0.0f);
		else
			clearColor = nvgRGBAf(0.3f, 0.3f, 0.32f, 1.0f);
		nvgClearWithColor(vg, clearColor);

		nvgBeginFrame(vg, winWidth, winHeight, pxRatio);
			renderDemo(vg, mx,my, winWidth,winHeight, t, blowup, &data);
			renderGraph(vg, 5,5, &fps);
			renderGraph(vg, 5+200+5,5, &cpuGraph);
		nvgEndFrame(vg);

		updateGraph(&fps, dt);
        cpuTime = (SDL_GetTicks() / 1e3) - t;
		updateGraph(&cpuGraph, cpuTime);

		SDL_GL_SwapWindow(window);
	}
	printf("Average Frame Time: %.2f ms\n", getGraphAverage(&fps) * 1000.0f);
	printf("          CPU Time: %.2f ms\n", getGraphAverage(&cpuGraph) * 1000.0f);

	return EXIT_SUCCESS;
}
