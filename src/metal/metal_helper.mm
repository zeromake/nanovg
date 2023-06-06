#include "metal_helper.h"
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#import <AppKit/AppKit.h>

struct MetalContext {
    id<MTLDevice> device;
    CAMetalLayer* layer;
    NSWindow* win;
};

extern "C" void *GetMetalLayer(struct MetalContext *ctx) {
    return (__bridge void *)ctx->layer;
}

extern "C" void DestroyMetalContext(struct MetalContext *ctx) {
    free(ctx);
}

extern "C" struct MetalContext *CreateMetalContext(void* window) {
    struct MetalContext* mtl = (struct MetalContext*)calloc(1, sizeof(struct MetalContext));
    NSWindow* nswin = (__bridge NSWindow*)window;
    mtl->win = nswin;
    mtl->device = MTLCreateSystemDefaultDevice();
    mtl->layer = [CAMetalLayer layer];
    mtl->layer.device = mtl->device;
    mtl->layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    nswin.contentView.layer = mtl->layer;
    nswin.contentView.wantsLayer = YES;
    return mtl;
}

extern "C" void ResizeMetalDrawable(struct MetalContext *ctx, int width, int height) {
    CGSize sz;
    sz.width = width;
    sz.height = height;
    ctx->layer.drawableSize = sz;
}

float GetMetalScaleFactor(struct MetalContext *ctx) {
    float scaleFactor = [ctx->win backingScaleFactor];
    return scaleFactor;
}
