#ifndef D3D11_HELPER_H
#define D3D11_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <d3d11.h>

typedef struct D3D11Context D3D11Context;

// 创建一个 d3d 上下文
D3D11Context *CreateD3D11Context(void *window, int width, int height);


// 从 d3d 上下文里取出 D3D11Device
void *GetD3D11Device(D3D11Context *ctx);

// 销毁 d3d 上下文
void DestroyD3D11Context(D3D11Context *ctx);

// 重设 d3d 绘制大小
bool ResizeD3D11Drawable(D3D11Context *ctx, int width, int height);

// 获取窗口 dpi 缩放比例
float GetD3D11ScaleFactor(D3D11Context *ctx);

// 使用颜色清空绘制区域
void ClearD3D11WithColor(D3D11Context *ctx, float clearColor[4]);

// 将 SwapChain 里的绘制到窗口
void D3D11Present(D3D11Context *ctx);

ID3D11Texture2D* D3D11GetSwapChainTexture(D3D11Context *ctx);

#ifdef __cplusplus
}
#endif

#endif
