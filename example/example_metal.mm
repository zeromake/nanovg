#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

#include <nanovg.h>
#include "nanovg_mtl.h"
#include "demo.h"
#include "perf.h"

struct context_mtl {
	id<MTLDevice> device;
	CAMetalLayer* layer;
};

int blowup = 0;
int screenshot = 0;
int premult = 0;

static void key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	NVG_NOTUSED(scancode);
	NVG_NOTUSED(mods);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		blowup = !blowup;
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		screenshot = 1;
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
		premult = !premult;
}

int main(int argc, const char * argv[]) {
	GLFWwindow* window;
	DemoData data;
	NVGcontext* vg = NULL;
	double prevt = 0, cpuTime = 0;
	PerfGraph fps, cpuGraph;
	initGraph(&fps, GRAPH_RENDER_FPS, "Frame Time");
	initGraph(&cpuGraph, GRAPH_RENDER_MS, "CPU Time");
    struct context_mtl* mtl = (struct context_mtl*)calloc(1, sizeof(struct context_mtl));
    if (!glfwInit()) {
		printf("Failed to init GLFW.");
		return -1;
	}
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(1000, 600, "NanoVG", NULL, NULL);
    NSWindow* nswin = glfwGetCocoaWindow(window);
    mtl->device = MTLCreateSystemDefaultDevice();
    mtl->layer = [CAMetalLayer layer];
    mtl->layer.device = mtl->device;
    mtl->layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    nswin.contentView.layer = mtl->layer;
    nswin.contentView.wantsLayer = YES;

    float scaleFactor = [nswin backingScaleFactor];

    vg = nvgCreateMTL((__bridge void *)mtl->layer, NVG_ANTIALIAS | NVG_STENCIL_STROKES);
    // int fb_width, fb_height;
    // glfwGetFramebufferSize(window, &fb_width, &fb_height);
    // CGSize sz;
    // sz.width = fb_width;
    // sz.height = fb_height;
    // mtl->layer.drawableSize = sz;

    glfwSetKeyCallback(window, key);
	glfwMakeContextCurrent(window);

    
	if (loadDemoData(vg, &data) == -1)
		return -1;
    
	glfwSwapInterval(0);
    glfwSetTime(0);
	prevt = glfwGetTime();
    int prevWinWidth = 0,prevWinHeight = 0;
    while (!glfwWindowShouldClose(window))
	{
		double mx, my, t, dt;
		int winWidth, winHeight;
		float gpuTimes[3];
		int i, n;

		t = glfwGetTime();
		dt = t - prevt;
		prevt = t;

		glfwGetCursorPos(window, &mx, &my);
		glfwGetWindowSize(window, &winWidth, &winHeight);
        if (prevWinWidth != winWidth || prevWinHeight != winHeight) {
            int fb_width, fb_height;
            glfwGetFramebufferSize(window, &fb_width, &fb_height);
            CGSize sz;
            sz.width = fb_width;
            sz.height = fb_height;
            mtl->layer.drawableSize = sz;
        }
        prevWinWidth = winWidth;
        prevWinHeight = winHeight;

		if (premult)
			mnvgClearWithColor(vg, nvgRGBAf(0.0f, 0.0f, 0.0f, 1.0f));
		else
			mnvgClearWithColor(vg, nvgRGBAf(0.3f, 0.3f, 0.32f, 1.0f));

		nvgBeginFrame(vg, winWidth, winHeight, scaleFactor);

		renderDemo(vg, mx,my, winWidth, winHeight, t, blowup, &data);

		renderGraph(vg, 5,5, &fps);
		renderGraph(vg, 5+200+5,5, &cpuGraph);
		nvgEndFrame(vg);

		updateGraph(&fps, dt);
        cpuTime = glfwGetTime() - t;
		updateGraph(&cpuGraph, cpuTime);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	printf("Average Frame Time: %.2f ms\n", getGraphAverage(&fps) * 1000.0f);
	freeDemoData(vg, &data);
    nvgDeleteMTL(vg);
	glfwTerminate();
	return 0;  
}
