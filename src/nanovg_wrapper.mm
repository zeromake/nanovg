#import <AppKit/AppKit.h>

float GetScaleFactor(void* win) {
    NSWindow* window = (__bridge NSWindow*)win;
    float scaleFactor = [window backingScaleFactor];
    return scaleFactor;
}
