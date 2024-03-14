#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_log.h>
#include <sokol_glue.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define NANOVG_SOKOL_IMPLEMENTATION 1
#include <nanovg_sokol.h>

#include "frequency.h"
#include "demo.h"

typedef struct State {
    sg_pass_action pass_action;
    NVGcontext *vg;
    DemoData data;
    float scaleFactor;
    float mouse[2];
} State;

static void init(void* ptr) {
    State* state = (State*)ptr;
    state->scaleFactor = sapp_dpi_scale();
    nvgFrequencyInitTimer();
    sg_setup(&(sg_desc){
        .context = sapp_sgcontext(),
        .logger.func = slog_func,
        .pipeline_pool_size = 1024,
    });
    state->vg = nvgCreateSokol(NVG_ANTIALIAS|NVG_STENCIL_STROKES);
    assert(state->vg != NULL);
    // NVGcolor bgColor = nvgRGBA(0xef, 0xe6, 0xc7, 255);
    NVGcolor bgColor = nvgRGBAf(0.3f, 0.3f, 0.32f, 1.0f);
    state->pass_action = (sg_pass_action) {
        .colors[0] = {
            .load_action=SG_LOADACTION_CLEAR,
            .clear_value={bgColor.r, bgColor.g, bgColor.b, bgColor.a},
        }
    };
	loadDemoData(state->vg, &state->data);
}

static void demo1(State* state) {
    NVGcontext *vg = state->vg;
    nvgBeginPath(vg);
    nvgRect(vg, 100, 100, 300, 150);
    nvgFillColor(vg, nvgRGBAf(1, 0, 0, 0.5));
    nvgFill(vg);
    nvgClosePath(vg);
}

static void demo2(State* state) {
    double t = nvgFrequencyGetTime();
    nvgScale(state->vg, state->scaleFactor, state->scaleFactor);
    renderDemo(state->vg, state->mouse[0], state->mouse[1], sapp_width()/2, sapp_height()/2, t, 0, &state->data);
}

static void frame(void* ptr) {
    State* state = (State*)ptr;
    sg_begin_default_pass(&state->pass_action, sapp_width(), sapp_height());

    nvgBeginFrame(state->vg, sapp_width(), sapp_height(), state->scaleFactor);
    demo1(state);
    nvgEndFrame(state->vg);

    sg_end_pass();
    sg_commit();
}

static void cleanup(void* ptr) {
    State* state = (State*)ptr;
    nvgDeleteSokol(state->vg);
    state->vg = NULL;
    sg_shutdown();
    free(state);
}


static void event(const sapp_event* e, void* ptr) {
    State* state = (State*)ptr;
    int x = 0;
    int y = 0;
    switch (e->type) {
        case SAPP_EVENTTYPE_KEY_DOWN:
            if (e->key_code == SAPP_KEYCODE_Q) {
                sapp_request_quit();
            }
            break;
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            state->mouse[0] = e->mouse_x / state->scaleFactor;
            state->mouse[1] = e->mouse_y / state->scaleFactor;
            break;
    }
}

sapp_desc sokol_main(int argc, char* argv[]) {
    State* app = (State*)malloc(sizeof(State));
    memset(app, 0, sizeof(State));
    return (sapp_desc){
        .init_userdata_cb = init,
        .frame_userdata_cb = frame,
        .cleanup_userdata_cb = cleanup,
        .event_userdata_cb = event,
        .user_data = app,
        .width = 800,
        .height = 600,
        .window_title = "NanoVG sokol",
        .logger.func = slog_func,
        .high_dpi = true,
        .swap_interval = 1,
        .win32_console_utf8 = true,
    };
}
