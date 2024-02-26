#ifndef DEMO_H
#define DEMO_H

#include "nanovg.h"

#ifdef __cplusplus
extern "C" {
#endif

struct DemoData {
	int fontNormal, fontBold, fontIcons, fontEmoji, fontCJK;
	int images[12];
    int maskImage;
};
typedef struct DemoData DemoData;

int loadDemoData(NVGcontext* vg, DemoData* data);
void freeDemoData(NVGcontext* vg, DemoData* data);
void renderDemo(NVGcontext* vg, float mx, float my, float width, float height, float t, int blowup, DemoData* data);

void saveScreenShot(int w, int h, int premult, const char* name);
void drawParagraph(NVGcontext* vg, float x, float y, float width, float height, float mx, float my);

#ifdef __cplusplus
}
#endif

#endif // DEMO_H
