// Build with: gcc -o main_sw main_sw.c `pkg-config --libs --cflags mpv sdl2` -std=c99

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <nanovg_d3d11.h>
#include <SDL.h>
#include <mpv/client.h>
#include <mpv/render.h>
#include "d3d11.hpp"
#include <memory>

static Uint32 wakeup_on_mpv_render_update, wakeup_on_mpv_events;

static void die(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

static void on_mpv_events(void *ctx)
{
    SDL_Event event = {wakeup_on_mpv_events};
    SDL_PushEvent(&event);
}

static void on_mpv_render_update(void *ctx)
{
    SDL_Event event = {wakeup_on_mpv_render_update};
    SDL_PushEvent(&event);
}

#undef main
int main(int argc, char *argv[])
{
    if (argc != 2)
        die("pass a single media file as argument");

    mpv_handle *mpv = mpv_create();
    if (!mpv)
        die("context init failed");

    // Some minor options can only be set before mpv_initialize().
    if (mpv_initialize(mpv) < 0)
        die("mpv init failed");

    mpv_request_log_messages(mpv, "debug");
    mpv_set_option_string(mpv, "hwdec", "auto");

    // Jesus Christ SDL, you suck!
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "no");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11");

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        die("SDL init failed");

    SDL_Window *window;
    int windowWidth = 1024;
    int windowHeight = 800;
    window = SDL_CreateWindow(
		"SDL2/D3D11/NanoVG",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		windowWidth,
		windowHeight,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
	);
    if (!window)
        die("failed to create SDL window");

    int control = 1;
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE, MPV_RENDER_API_TYPE_SW},
        // Tell libmpv that you will call mpv_render_context_update() on render
        // context update callbacks, and that you will _not_ block on the core
        // ever (see <libmpv/render.h> "Threading" section for what libmpv
        // functions you can call at all when this is active).
        // In particular, this means you must call e.g. mpv_command_async()
        // instead of mpv_command().
        // If you want to use synchronous calls, either make them on a separate
        // thread, or remove the option below (this will disable features like
        // DR and is not recommended anyway).
        {MPV_RENDER_PARAM_ADVANCED_CONTROL, &control},
        {0}
    };

    mpv_render_context *mpv_rd;
    if (mpv_render_context_create(&mpv_rd, mpv, params) < 0)
        die("failed to initialize mpv render context");

    // We use events for thread-safe notification of the SDL main loop.
    // Generally, the wakeup callbacks (set further below) should do as least
    // work as possible, and merely wake up another thread to do actual work.
    // On SDL, waking up the mainloop is the ideal course of action. SDL's
    // SDL_PushEvent() is thread-safe, so we use that.
    wakeup_on_mpv_render_update = SDL_RegisterEvents(1);
    wakeup_on_mpv_events = SDL_RegisterEvents(1);
    if (wakeup_on_mpv_render_update == (Uint32)-1 ||
        wakeup_on_mpv_events == (Uint32)-1)
        die("could not register events");

    // When normal mpv events are available.
    mpv_set_wakeup_callback(mpv, on_mpv_events, NULL);

    // When there is a need to call mpv_render_context_update(), which can
    // request a new frame to be rendered.
    // (Separate from the normal event handling mechanism for the sake of
    //  users which run OpenGL on a different thread.)
    mpv_render_context_set_update_callback(mpv_rd, on_mpv_render_update, NULL);


    // init nanovg
    auto D3D11_CONTEXT = std::make_shared<brls::D3D11Context>();
    if (!D3D11_CONTEXT->InitializeDX(window, windowWidth, windowHeight)) {
        die("failed to initialize nanovg");
    }
    NVGcontext* nvgContext = nvgCreateD3D11(D3D11_CONTEXT->GetDevice(), NVG_ANTIALIAS | NVG_STENCIL_STROKES);

    SDL_Texture *tex = NULL;
    int nvg_image = 0;
    int tex_w = -1, tex_h = -1;

    // Play this file.
    const char *cmd[] = {"loadfile", argv[1], NULL};
    mpv_command_async(mpv, 0, cmd);

    void* pixels = nullptr;
    while (1) {
        SDL_Event event;
        if (SDL_WaitEvent(&event) != 1)
            die("event loop error");
        int redraw = 0;
        switch (event.type) {
        case SDL_QUIT:
            goto done;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_EXPOSED)
                redraw = 1;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_SPACE) {
                const char *cmd_pause[] = {"cycle", "pause", NULL};
                mpv_command_async(mpv, 0, cmd_pause);
            }
            if (event.key.keysym.sym == SDLK_s) {
                // Also requires MPV_RENDER_PARAM_ADVANCED_CONTROL if you want
                // screenshots to be rendered on GPU (like --vo=gpu would do).
                const char *cmd_scr[] = {"screenshot-to-file",
                                         "screenshot.png",
                                         "window",
                                         NULL};
                printf("attempting to save screenshot to %s\n", cmd_scr[1]);
                mpv_command_async(mpv, 0, cmd_scr);
            }
            break;
        default:
            // Happens when there is new work for the render thread (such as
            // rendering a new video frame or redrawing it).
            if (event.type == wakeup_on_mpv_render_update) {
                uint64_t flags = mpv_render_context_update(mpv_rd);
                if (flags & MPV_RENDER_UPDATE_FRAME)
                    redraw = 1;
            }
            // Happens when at least 1 new event is in the mpv event queue.
            if (event.type == wakeup_on_mpv_events) {
                // Handle all remaining mpv events.
                while (1) {
                    mpv_event *mp_event = mpv_wait_event(mpv, 0);
                    if (mp_event->event_id == MPV_EVENT_NONE)
                        break;
                }
            }
        }
        if (redraw) {
            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            if (!pixels || tex_w != w || tex_h != h) {
                D3D11_CONTEXT->ResizeFramebufferSize(w, h);
                if (nvg_image) {
                    nvgDeleteImage(nvgContext, nvg_image);
                    if (pixels) free(pixels);
                }
                pixels = malloc(w * h * 4);
                nvg_image = nvgCreateImageRGBA(nvgContext, w, h, NVG_IMAGE_STREAMING|NVG_IMAGE_COPY_SWAP, (const unsigned char *)pixels);
                if (!pixels) {
                    printf("could not allocate texture\n");
                    exit(1);
                }
                tex_w = w;
                tex_h = h;
            }
            int size[] = {w, h};
            size_t pitch = w * 4;
            mpv_render_param params[] = {
                {MPV_RENDER_PARAM_SW_SIZE, &size[0]},
                {MPV_RENDER_PARAM_SW_FORMAT, "rgba"},
                {MPV_RENDER_PARAM_SW_STRIDE, &pitch},
                {MPV_RENDER_PARAM_SW_POINTER, pixels},
                {0}
            };
            int r = mpv_render_context_render(mpv_rd, params);
            if (r < 0) {
                printf("mpv_render_context_render error: %s\n",
                       mpv_error_string(r));
                exit(1);
            }
            D3D11_CONTEXT->ClearWithColor(NVGcolor{0.0f, 0.0f, 0.0f, 1.0f});
            nvgBeginFrame(nvgContext, w, h, 2.0f);
            nvgUpdateImage(nvgContext, nvg_image, (const unsigned char *)pixels);
            nvgBeginPath(nvgContext);
            nvgRect(nvgContext, 0, 0, w, h);
            nvgFillPaint(nvgContext, nvgImagePattern(nvgContext, 0, 0, w, h, 0, nvg_image, 1.0f));
            nvgFill(nvgContext);
            nvgEndFrame(nvgContext);
            D3D11_CONTEXT->Present();
        }
    }
done:
    nvgDeleteD3D11(nvgContext);
    D3D11_CONTEXT = nullptr;

    // Destroy the GL renderer and all of the GL objects it allocated. If video
    // is still running, the video track will be deselected.
    mpv_render_context_free(mpv_rd);

    mpv_destroy(mpv);

    printf("properly terminated\n");
    return 0;
}