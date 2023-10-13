#include "metal_helper.h"
#include "nanovg.h"
#include "nanovg_mtl.h"
#import <simd/simd.h>
#include <string>
#include <stdio.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#import <AppKit/AppKit.h>

struct MetalContext {
    id<MTLDevice> device;
    CAMetalLayer* layer;
    NSWindow* win;
    NVGrendererInfo renderInfo;
};

void *GetMetalLayer(struct MetalContext *ctx) {
    return (__bridge void *)ctx->layer;
}

void DestroyMetalContext(struct MetalContext *ctx) {
    free(ctx);
}

#if TARGET_OS_IOS
static void GetFeatureSetsForIOS(const MTLFeatureSet*& fsets, std::size_t& count, MTLFeatureSet& fsetDefault)
{
    static const MTLFeatureSet g_featureSetsIOS[] =
    {
        16,
        15,
        14,
        13,
        12,

        11,
        10,
        9,
        8,

        7,
        6,
        5,

        4,
        3,
        2,

        1,
        0,
    };

    fsets       = g_featureSetsIOS;
    count       = sizeof(g_featureSetsIOS)/sizeof(g_featureSetsIOS[0]);
    fsetDefault = 0;
}

#else

static void GetFeatureSetsForMacOS(const MTLFeatureSet*& fsets, std::size_t& count, MTLFeatureSet& fsetDefault)
{
    static const MTLFeatureSet g_featureSetsMacOS[] =
    {
        10005,
        10004,
        10003,
        10002,
        10001,
        10000,
    };

    fsets       = g_featureSetsMacOS;
    count       = sizeof(g_featureSetsMacOS)/sizeof(g_featureSetsMacOS[0]);
    fsetDefault = 10000;
}

#endif // TARGET_OS_IOS

static MTLFeatureSet QueryHighestFeatureSet(id<MTLDevice> device_)
{
    /* Get list of feature sets for macOS or iOS */
    const MTLFeatureSet* fsets;
    std::size_t count;
    MTLFeatureSet fsetDefault;
    #if TARGET_OS_IOS
    GetFeatureSetsForIOS(fsets, count, fsetDefault);
    #else
    GetFeatureSetsForMacOS(fsets, count, fsetDefault);
    #endif
    /* Find highest supported feature set */
    for (std::size_t i = 0; i < count; ++i)
    {
        if ([device_ supportsFeatureSet:fsets[i]])
            return fsets[i];
    }

    return fsetDefault;
}


const char* QueryMetalVersion(id<MTLDevice> device_)
{
    const auto featureSet = QueryHighestFeatureSet(device_);

    #if TARGET_OS_IOS
    switch (featureSet)
    {
        case 16: return "GPU Family 5.v1";
        case 15: return "GPU Family 4.v2";
        case 14: return "GPU Family 3.v4";
        case 13: return "GPU Family 2.v5";
        case 12: return "GPU Family 1.v5";

        case 11: return "GPU Family 4.v1";
        case 10: return "GPU Family 3.v3";
        case 9: return "GPU Family 2.v4";
        case 8: return "GPU Family 1.v4";

        case 7: return "GPU Family 3.v2";
        case 6: return "GPU Family 2.v3";
        case 5: return "GPU Family 1.v3";

        case 4: return "GPU Family 3.v1";
        case 3: return "GPU Family 2.v2";
        case 2: return "GPU Family 1.v2";

        case 1: return "GPU Family 2.v1";
        case 0: return "GPU Family 1.v1";
    }

    #else

    switch (featureSet)
    {
        case 10005: return "2.1";
        case 10004: return "1.4";
        case 10003: return "1.3";
        case 10002: return "1.2.1";
        case 10001: return "1.2";
        case 10000: return "1.1";
        default: break;
    }
    #endif
    return "1.0";
}

struct MetalContext *CreateMetalContext(void* window) {
    struct MetalContext* mtl = (struct MetalContext*)calloc(1, sizeof(struct MetalContext));
    NSWindow* nswin = (__bridge NSWindow*)window;
    mtl->win = nswin;
    mtl->device = MTLCreateSystemDefaultDevice();
    mtl->layer = [CAMetalLayer layer];
    mtl->layer.device = mtl->device;
    mtl->layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    mtl->layer.framebufferOnly = false;
    nswin.contentView.layer = mtl->layer;
    nswin.contentView.wantsLayer = YES;
    char name[256] = {0};
    sprintf(name, "Metal %s", QueryMetalVersion(mtl->device));
    strcat(mtl->renderInfo.rendererName, name);
    strcat(mtl->renderInfo.deviceName, [[mtl->device name] cStringUsingEncoding:NSUTF8StringEncoding]);
    strcat(mtl->renderInfo.vendorName, "Apple");
    strcat(mtl->renderInfo.shaderName, "Metal Shading Language");
    return mtl;
}

void ResizeMetalDrawable(struct MetalContext *ctx, int width, int height) {
    CGSize sz;
    sz.width = width;
    sz.height = height;
    ctx->layer.drawableSize = sz;
}

float GetMetalScaleFactor(struct MetalContext *ctx) {
    float scaleFactor = [ctx->win backingScaleFactor];
    return scaleFactor;
}

NVGrendererInfo MetalGetRenderInfo(MetalContext *ctx) {
    return ctx->renderInfo;
}
// vector_uint2 mnvgCurrentTextureSize(NVGcontext* ctx);

int GetMetalScreenshotTexture(NVGcontext* ctx, int w, int h) {
    // vector_uint2 textureSize = mnvgCurrentTextureSize(ctx);
    int image = nvgCreateImageBGRA(ctx, w, h, NVG_IMAGE_PREMULTIPLIED, NULL);
    mnvgCopyCurrentTexture(ctx, image);
    return image;
}
