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

#if defined(__ANDROID__) && !defined(ANDROID)
#define ANDROID
#endif

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
#define NANOVG_USE_D3D11 1
// #define NANOVG_USE_GL 1
// #define NANOVG_USE_GL3 1
// #ifdef NANOVG_GLEW
// #include <GL/glew.h>
// #endif
#elif defined(__APPLE__)
#define NANOVG_USE_METAL 1
#elif defined(__linux__)
#define NANOVG_USE_GL 1
#define NANOVG_USE_GL3 1
#ifdef NANOVG_GLEW
#include <GL/glew.h>
#endif
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
#define DP(px) (int)((float)px * fbRatio)
// #define USE_FPS

#ifdef USE_FPS
#include "perf.h"
#include "perf.c"
#endif

static const double defaultAnimationSpeed = 0.3;

double GetElapsedTime(Uint64 start, double frequency) {
    return (double)(SDL_GetPerformanceCounter() - start) / frequency;
}

NVGScreenshotTexture* renderTextPattern(
    NVGcontext* vg,
    char * text,
    int winWidth,
    int winHeight,
    int fbWidth,
    int fbHeight,
    float fbRatio
) {
    NVGcolor bgColor = nvgRGBA(0xef, 0xe6, 0xc7, 255);
    nvgClearWithColor(vg, bgColor);
    nvgBeginFrame(vg, winWidth, winHeight, fbRatio);

    const char* start;
    const char* end;
    int nrows;
#ifdef ANDROID
    float size = DP(48);
#else
    float size = DP(24);
#endif
    int x = size;
    int y = size;
    float lineh = 0;
    nvgFillColor(vg, nvgRGBA(0x49,0x43,0x30,255));
    nvgFontSize(vg, size);
    nvgFontFace(vg, "sans");
    nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
    nvgTextMetrics(vg, NULL, NULL, &lineh);

    NVGtextRow rows[3];
    start = text;
    end = text + strlen(text);
    while ((nrows = nvgTextBreakLines(vg, start, end, winWidth - (size * 2), rows, 3))) {
        for (int i = 0; i < nrows; i++) {
            NVGtextRow* row = &rows[i];
            nvgText(vg, x, y, row->start, row->end);
            y += lineh;
            if (y > (winHeight - size)) {
                goto loop;
            }
        }
        start = rows[nrows-1].next;
    }
    loop:
    nvgEndFrame(vg);
    return nvgScreenshotTexture(vg, 0, 0, fbWidth, fbHeight);
}


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
    SDL_GL_SetSwapInterval(-1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

#ifdef NANOVG_USE_GLES2
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#elif defined(NANOVG_USE_GLES3)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);

#if defined(ANDROID) && __ANDROID_API__ >= 24
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#elif defined(ANDROID) && __ANDROID_API__ >= 21
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    // opengl 自动选择版本
	// SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	// SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
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
    windowflags |= SDL_WINDOW_RESIZABLE;
#ifdef ANDROID
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "Portrait");
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
    const GLubyte * OpenGLVersion = glGetString(GL_VERSION);
    const GLubyte * name = glGetString(GL_RENDERER);
    printf("opengl: %s => %s\n", (char*)name, (char*)OpenGLVersion);
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
    const char* text2 = "「精王斧——傲斩」\n"
    "铸造者：瀚贝瑞·奎罗特\n"
    "点化者：哈德克\n"
    "现今持有者：利斯顿·奎罗特\n"
    "这柄古朴的巨大战斧是由白石厅国王，瀚贝瑞·奎罗特亲自以魔法金属奥利哈康打造的。"
    "曾受瀚贝瑞救命之恩的大法师哈德克自奋告勇地为这柄上好的利刃加持魔法，在十四名顶级法师连续运做法阵一周后，寒冰斧傲斩诞生了。\n"
    "然而，由于瀚贝瑞·奎罗特的另一个身份是白石厅两大家族之一奎罗特家族的族长，因此，长期不合的索瑞森家族对这柄武器嗤之以鼻，"
    "称它是“一把愚蠢的菜刀”，然而他们谁都没有料到，对头打造的武器居然会在危机到来时成为最亲密的战友。\n"
    "当炎龙美露基狄克试图摧毁月之都时，邻近区域外围的白石厅一度遭遇灭顶之灾。在那段最艰难的时期里，"
    "傲斩的主人瀚贝瑞·奎罗特一直与云耀的主人雅加西并肩作战，没有后退过哪怕一步。最终，矮人们和精灵族联合起来，"
    "终于帮助皇帝战胜了火焰之王，可是瀚贝瑞却也因此长眠在了白石厅的废墟之下，再也无法睁开眼睛。\n"
    "与美露基狄克的战斗标志着精灵国度与白石厅矮人从此走向没落，可是白石矮人们却并没有消亡，他们最后走出了山脉，"
    "融入整个大陆之中。而继承了精王斧傲斩的奎罗特子孙们，依然象他们的祖先一样，以鲜血和生命捍卫着家族的尊严与荣耀。\n"
    "现在，这柄拥有驾驭寒冰之力的战斧属于利斯顿·奎罗特。";

    bool change = true;
    int prevW = fbWidth;
    int prevH = fbHeight;
    float fbRatio = nvgDevicePixelRatio(vg);
    printf("fbRatio: %f\n", fbRatio);

    // void* framebufferCurrnet = nvgCreateFramebuffer(vg, prevW, prevH, 0);
    // void* framebufferNext = nvgCreateFramebuffer(vg, prevW, prevH, 0);
    NVGScreenshotTexture* screenshotCurrnet = NULL;
    NVGScreenshotTexture* screenshotNext = NULL;
    bool show = false;
    bool reRenderText = true;
    bool isFullscreen = false;
    Uint64 start = SDL_GetPerformanceCounter();
    double frequency = (double)SDL_GetPerformanceFrequency();
    double showT = 0;


    bool nextChange = false;
    bool displayEvent = false;

#ifdef USE_FPS
    PerfGraph fps;
	initGraph(&fps, GRAPH_RENDER_FPS, "Frame Time");
#endif
	double prevt = GetElapsedTime(start, frequency);
    while (!quit) {
		double t = GetElapsedTime(start, frequency);
		double dt = t - prevt;
		prevt = t;
        SDL_PollEvent(&event);
        nextChange = false;
        switch(event.type) {
            case SDL_MOUSEBUTTONUP:
                show = !show;
                if (show) {
                    showT = GetElapsedTime(start, frequency);
                }
                change = true;
                break;
            case SDL_QUIT:
                quit=1;
                break;
            case SDL_KEYUP:
                if(event.key.keysym.scancode == SDL_SCANCODE_F11) {
                    SDL_SetWindowFullscreen(window, isFullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
                    isFullscreen = !isFullscreen;
                }
                // else if (event.key.keysym.scancode == SDL_SCANCODE_S && event.key.keysym.mod & KMOD_CTRL) {
                //     NVGScreenshotTexture* tex = nvgScreenshotTexture(vg, 0, 0, fbWidth, fbHeight);
                //     nvgScreenshotSave(vg, tex, "save.png");
                // }
                break;
#ifdef ANDROID
            case SDL_DISPLAYEVENT:
                displayEvent = true;
                break;
#endif
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_MOVED:
                    case SDL_WINDOWEVENT_EXPOSED:
                        // 最大化恢复
                        break;
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        // 窗口焦点获得
                    case SDL_APP_DIDENTERFOREGROUND:
                        // 应用回到前台
                        change = true;
                        break;
                    case SDL_APP_WILLENTERBACKGROUND:
                        // 应用后台
                        break;
                    case SDL_WINDOWEVENT_LEAVE:
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        // 失去焦点
                        break;
                    case SDL_WINDOWEVENT_RESIZED:
                        // 尺寸变化
                        break;
                    case SDL_WINDOWEVENT_DISPLAY_CHANGED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        // 窗口大小变化完成
                        SDL_GetWindowSize(window, &winWidth, &winHeight);
                        SDL_GetWindowSizeInPixels(window, &fbWidth, &fbHeight);
                        fbRatio = nvgDevicePixelRatio(vg);
                        nvgResetFrameBuffer(vg, fbWidth, fbHeight);
                        if (screenshotCurrnet != NULL) {
                            nvgDeleteImage(vg, screenshotCurrnet->image);
                            if (screenshotCurrnet->pixel) free(screenshotCurrnet->pixel);
                            free(screenshotCurrnet);
                            screenshotCurrnet = NULL;
                        }
                        if (screenshotNext != NULL) {
                            nvgDeleteImage(vg, screenshotNext->image);
                            if (screenshotNext->pixel) free(screenshotNext->pixel);
                            free(screenshotNext);
                            screenshotNext = NULL;
                        }
                        reRenderText = true;
                        printf("resize fb: %dx%d %dx%d %f\n", winWidth, winHeight, fbWidth, fbHeight, fbRatio);
                        change = true;
#ifdef ANDROID
                        if (displayEvent) {
                            displayEvent = false;
                            nextChange = true;
                        }
#endif
                        break;
                    default:
                        printf("window event %d\n", event.window.event);
                        break;
                }
                break;
            case SDL_MOUSEMOTION:
            case SDL_POLLSENTINEL:
                break;
            default:
                printf("event %d\n", event.type);
                break;
        }

        if (change) {
            // Update and render
            if (reRenderText) {
                printf("update: %d\n", event.window.event);
                screenshotCurrnet = renderTextPattern(vg, text, winWidth, winHeight, fbWidth, fbHeight, fbRatio);
                // if (show) {
                screenshotNext = renderTextPattern(vg, text2, winWidth, winHeight, fbWidth, fbHeight, fbRatio);
                // }
                printf("update screenshot: %d %d\n", screenshotCurrnet->image, screenshotNext->image);
                reRenderText = false;
            }

            nvgClearWithColor(vg, bgColor);
            nvgBeginFrame(vg, winWidth, winHeight, fbRatio);
            if (show) {
                NVGpaint img = nvgImagePattern(vg, 0, 0, winWidth, winHeight, 0, screenshotNext->image, 1.0f);
                nvgSave(vg);

                nvgBeginPath(vg);
                nvgRect(vg, 0, 0, winWidth, winHeight);
                nvgFillPaint(vg, img);
                nvgFill(vg);
                nvgRestore(vg);

                if (t <= (showT + defaultAnimationSpeed)) {
                    int offset = (int)((float)winWidth * ((t - showT) / defaultAnimationSpeed));
                    img = nvgImagePattern(vg, 0, 0, winWidth, winHeight, 0, screenshotCurrnet->image, 1.0f);
                    nvgSave(vg);
                    nvgTranslate(vg, -offset, 0);

                    nvgBeginPath(vg);
                    nvgRect(vg, 0, 0, winWidth, winHeight);
                    nvgFillPaint(vg, img);
                    nvgFill(vg);
                    NVGpaint headerPaint = nvgLinearGradient(vg, winWidth,0,winWidth+15,0, nvgRGBA(11, 11, 11, 66), nvgRGBA(0, 0, 0, 0));
                    nvgBeginPath(vg);
                    nvgRect(vg, winWidth, 0, 15, winHeight);
                    nvgFillPaint(vg, headerPaint);
                    nvgFill(vg);

                    nvgRestore(vg);
                    nextChange = true;
                }
            } else {
                NVGpaint img = nvgImagePattern(vg, 0, 0, winWidth, winHeight, 0, screenshotCurrnet->image, 1.0f);
                nvgSave(vg);

                nvgBeginPath(vg);
                nvgRect(vg, 0, 0, winWidth, winHeight);
                nvgFillPaint(vg, img);
                nvgFill(vg);
                nvgRestore(vg);
            }
            nvgEndFrame(vg);
#ifdef USE_FPS
            float avg = getGraphAverage(&fps);
            printf("fps: %f\n", avg > 0 ? 1.0f / avg : 0);
		    updateGraph(&fps, dt);
#endif
            nvgPresent(vg);
            if (!nextChange) {
                change = false;
            }
        } else {
            SDL_Delay(16);
        }
    }
    nvgDelete(vg);
    SDL_DestroyWindow(window);
    return EXIT_SUCCESS;
}
