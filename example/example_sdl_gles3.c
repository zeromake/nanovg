//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef _WIN32
#include <io.h>
#define access _access
#else
#include <unistd.h>
#endif

#define TAG "nanovg"

#ifdef ANDROID
#define NANOVG_USE_GL 1
#include <android/log.h>
#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>
#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#elif __ANDROID_API__ >= 18
#include <GLES3/gl3.h>
#else
#include <GLES2/gl2.h>
#define NANOVG_USE_GLES2 1
#endif
#ifndef NANOVG_USE_GLES2
#define NANOVG_USE_GLES3 1
#endif
#elif defined(_WIN32)
#define NANOVG_USE_GL 1
#define NANOVG_USE_GL3 1
#ifdef NANOVG_GLEW
#include <GL/glew.h>
#endif
#elif defined(__APPLE__)
#define NANOVG_USE_METAL 1
#endif

#include <SDL2/SDL.h>
#include "nanovg.h"
#define NANOVG_IMPLEMENTATION
#include "nanovg_wrapper.h"

#ifdef ANDROID
#include <android/log.h>
#define LOG_TAG "com.zeromake.example"
#define printf(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#endif

#ifndef ANDROID
#undef main
#endif
int main(int argc, char **argv) {
    int flags = SDL_INIT_EVERYTHING & ~(SDL_INIT_TIMER | SDL_INIT_HAPTIC);
    if (SDL_Init(flags) < 0) {
        printf("ERROR: SDL_Init failed: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

#ifdef NANOVG_USE_GL
    SDL_GL_SetSwapInterval(1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

#ifdef NANOVG_USE_GLES2
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#elif defined(NANOVG_USE_GLES3)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
#if __ANDROID_API__ >= 24
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#elif __ANDROID_API__ >= 21
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,  SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#endif

    // Try with these GL attributes
    // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
    int windowflags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
#ifdef NANOVG_USE_GL
    windowflags |= SDL_WINDOW_OPENGL;
#endif
#ifndef ANDROID
    windowflags |= SDL_WINDOW_RESIZABLE;
#endif

    SDL_Window *window = SDL_CreateWindow(
        "NanoVG Example",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        1024,
        800,
        windowflags);

#ifdef NANOVG_USE_GL
    SDL_GLContext context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);
#endif
#ifdef NANOVG_GLEW
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK) {
		printf("Could not init glew.\n");
		return -1;
	}
#endif
    int createFlags = NVG_ANTIALIAS | NVG_STENCIL_STROKES;
    NVGcontext* vg = nvgCreate(createFlags, window);
    if (vg == NULL) {
        printf("ERROR: NanoVG init failed");
        return EXIT_FAILURE;
    }

    int winWidth = 0, winHeight = 0;
    SDL_GetWindowSize(window, &winWidth, &winHeight);
    int fbWidth = winWidth;
    int fbHeight = winHeight;
    SDL_GetWindowSizeInPixels(window, &fbWidth, &fbHeight);

    int quit=0;
    SDL_Event event;
    NVGcolor bgColor = nvgRGBA(0xef, 0xe6, 0xc7, 255);
    nvgResetFrameBuffer(vg, fbWidth, fbHeight);
    nvgClearWithColor(vg, bgColor);
#if defined(_WIN32)
	int fontNormal = nvgCreateFont(vg, "sans", "C:\\Windows\\Fonts\\msyh.ttc");
    int fontEmoji = nvgCreateFont(vg, "emoji", "C:\\Windows\\Fonts\\seguiemj.ttf");
    nvgAddFallbackFontId(vg, fontNormal, fontEmoji);
#elif defined(__APPLE__)
	int fontNormal = nvgCreateFont(vg, "sans", "/System/Library/Fonts/PingFang.ttc");
    int fontEmoji = nvgCreateFont(vg, "emoji", "/System/Library/Fonts/Apple Color Emoji.ttc");
    nvgAddFallbackFontId(vg, fontNormal, fontEmoji);
#elif defined(ANDROID)
    char* cjks[] = {
        "/system/fonts/NotoSansSC-Regular.otf",
        "/system/fonts/NotoSansCJK-Regular.ttc",
        "/system/fonts/DroidSansFallback.ttf",
        "/system/fonts/DroidSansChinese.ttf"
    };
    int fontCJK = 0;
    for (int i = 0; i < 3; i++) {
        char* cjkPath = cjks[i];
        if (access(cjkPath, 0) != -1) {
            fontCJK = nvgCreateFont(vg, "cjk", cjkPath);
            break;
        }
    }
    int fontNormal = nvgCreateFont(vg, "sans", "/system/fonts/Roboto-Regular.ttf");
    // int fontEmoji = nvgCreateFont(vg, "emoji","/system/fonts/NotoColorEmoji.ttf");
    nvgAddFallbackFontId(vg, fontNormal, fontCJK);
    // nvgAddFallbackFontId(vg, fontNormal, fontEmoji);
#endif
    const char* t = "😇";
    const char* text = "😇云耀可以称为所有精王武器中最具有传奇色彩的一柄——并不是因为它本身的力量，而是因为那个神话般的持有者。"
    "几百年前，雅加西所使用的武器是一柄名为“天驱”的软剑，为女神菲娜所赠。然而，由于莫巴帝的背叛，菲娜在与世界树融合之际惨遭毁灭的命运，"
    "追随她的血族们也大都死在使徒的剑下，从此一蹶不振。而随着女神香消玉殆，神剑中的力量也开始缓慢地衰竭。"
    "数百年之后的路维丝时代，传说中的皇帝遇到了精灵黎瑟西尔，而当他为了黎瑟不惜同时对抗三千名精灵族高阶战士时，“天驱”终于耗尽了最后一份力量，断为无数碎片。\n"
    "此战终结，皇帝不仅救下了黎瑟西尔，同时也赢得了白石厅矮人的尊重——矮人们决定为他重新铸造一柄武器。"
    "铸剑师们激烈的竞争下，最终白石厅两大家族之一的索瑞森家族靠着自身的实力以及少许运气，获得了这一权利。"
    "族长达斯特·索瑞森亲自在白石山脉最大的熔炉中以魔法金属奥利哈钢铸造出一柄锋利无比的软剑，祈祷士黎瑟西尔则为这柄武器加持了最适合皇帝的魔法——驾御风的能力。\n"
    "雅加西进攻的速度极快，薄薄的软剑在这种速度下很容易为空气阻力本身带偏，但云耀却不会。"
    "这柄剑就像是永远将自己包裹在真空中一般，无论周围刮起多大的狂风，它也绝对不会偏转一丝一毫，它将永远随着持有者的意志，如闪电般斩下。\n"
    "这柄剑就是后世无人不知的“云耀”，它同时也是黎瑟西尔与雅加西爱情的见证。\n"
    "皇帝最终因炎龙的诅咒而死在大卢尔德著名的热砂战场上，但武器却被传承给了他的手下败将——雷霆剑圣阿尔萨斯。"
    "这柄剑于是跟随着它的新主人默默无闻地走过整个卡那多斯，一直到了严寒的诺德森大陆，之后再也没有人见过云耀，直到伊修托利与路维丝的战争拉开序幕之时。\n";

    bool change = true;
    int prevW = fbWidth;
    int prevH = fbHeight;
    float fbRatio = nvgDevicePixelRatio(vg);

#define DP(px) (int)((float)px * fbRatio)
    while (!quit) {
        SDL_PollEvent(&event);
        switch(event.type) {
            case SDL_QUIT:
                quit=1;
                break;
			case SDL_WINDOWEVENT:
			    switch (event.window.event) {
                    case SDL_WINDOWEVENT_EXPOSED:
                        // 最大化恢复
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        // 窗口焦点获得
                    case SDL_APP_DIDENTERFOREGROUND:
                        // 应用回到前台
                        change = true;
                        break;
                    case SDL_APP_WILLENTERBACKGROUND:
                        // 应用后台
                        break;
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        // 失去焦点
                        break;
                    case SDL_WINDOWEVENT_RESIZED:
                        // 尺寸变化
                        SDL_GetWindowSize(window, &winWidth, &winHeight);
                        break;
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        // 窗口大小变化完成
                        SDL_GetWindowSizeInPixels(window, &fbWidth, &fbHeight);
                        fbRatio = nvgDevicePixelRatio(vg);
                        nvgResetFrameBuffer(vg, fbWidth, fbHeight);
                        change = true;
                        break;
                }
              break;
        }
        if (change) {
            // Update and render
            nvgClearWithColor(vg, bgColor);
            nvgBeginFrame(vg, winWidth, winHeight, fbRatio);

            const char* start;
            const char* end;
            int nrows;
            int x = DP(50);
            int y = DP(50);
            float lineh = 0;
            nvgFillColor(vg, nvgRGBA(0x49,0x43,0x30,255));
            nvgFontSize(vg, 48.0f);
            nvgFontFace(vg, "sans");
            nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
            nvgTextMetrics(vg, NULL, NULL, &lineh);

            NVGtextRow rows[3];
            start = text;
            end = text + strlen(text);
            while ((nrows = nvgTextBreakLines(vg, start, end, winWidth - DP(80), rows, 3))) {
                for (int i = 0; i < nrows; i++) {
                    NVGtextRow* row = &rows[i];
                    nvgText(vg, x, y, row->start, row->end);
                    y += lineh;
                    if (y > (winHeight - DP(50))) {
                        goto loop;
                    }
                }
                start = rows[nrows-1].next;
            }
            loop:
            nvgEndFrame(vg);
            SDL_GL_SwapWindow(window);
            change = false;
        }
        SDL_Delay(16);
    }
    nvgDelete(vg);
    SDL_DestroyWindow(window);
    return EXIT_SUCCESS;
}
