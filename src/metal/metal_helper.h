#ifndef METAL_HELPER_H
#define METAL_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MetalContext MetalContext;

// 创建一个 metal 上下文
MetalContext *CreateMetalContext(void* window);
// 销毁一个 metal 上下文
void DestroyMetalContext(MetalContext *ctx);
// 从 metal 上下文里取出 MetalLayer
void *GetMetalLayer(MetalContext *ctx);
// MetalLayer 画布缩放
void ResizeMetalDrawable(MetalContext *ctx, int width, int height);
// 获取窗口 dpi
float GetMetalScaleFactor(MetalContext *ctx);
#ifdef __cplusplus
}
#endif

#endif
