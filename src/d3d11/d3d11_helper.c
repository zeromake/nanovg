#include <windows.h>
#include <d3d11_1.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <stdio.h>

#include "d3d11_helper.h"

#ifdef __WINRT__
#include <winrt/Windows.Graphics.Display.h>
#else
__declspec(dllimport) UINT __stdcall GetDpiForWindow(HWND hwnd);
#endif

#ifdef __cplusplus
#define D3D_API(p, name, ...) p->name(__VA_ARGS__)
#define D3D_API_RELEASE(p)  \
    {                       \
        if ((p))            \
        {                   \
            (p)->Release(); \
            (p) = NULL;     \
        }                   \
    }
#else
#define D3D_API(p, name, ...) p->lpVtbl->name(p, __VA_ARGS__)
#define D3D_API_RELEASE(p)             \
    {                                  \
        if ((p))                       \
        {                              \
            (p)->lpVtbl->Release((p)); \
            (p) = NULL;                \
        }                              \
    }
#endif

// by dxguid.lib
// static const IID IID_IDXGIDevice = {0x54ec77fa,0x1377,0x44e6,{0x8c,0x32,0x88,0xfd,0x5f,0x44,0xc8,0x4c}};
// static const IID IID_IDXGIFactory2 = {0x50c83a1c,0xe072,0x4c48,{0x87,0xb0,0x36,0x30,0xfa,0x36,0xa6,0xd0}};
// static const IID IID_ID3D11Texture2D = {0x6f15aaf2,0xd208,0x4e89,{0x9a,0xb4,0x48,0x95,0x35,0xd3,0x4f,0x9c}};


bool _ResizeD3D11Drawable(D3D11Context *ctx, int width, int height, bool init);

struct D3D11Context
{
#ifdef __WINRT__
    IUnknown *coreWindow;
#else
    HWND hwnd;
#endif
    ID3D11Device *device;
    ID3D11DeviceContext *deviceContext;
    IDXGISwapChain1 *swapChain;
    ID3D11RenderTargetView *renderTargetView;
    ID3D11Texture2D *depthStencil;
    ID3D11DepthStencilView *depthStencilView;
    D3D_FEATURE_LEVEL featureLevel;
    DXGI_SAMPLE_DESC sampleDesc;
    NVGrendererInfo renderInfo;
};


void *GetD3D11Device(D3D11Context *ctx) {
    return (void*)ctx->device;
}

const char* GetVendorName(unsigned short vendor)
{
    switch (vendor)
    {
        case 0x106B:      return "Apple Inc.";
        case 0x1022:      return "Advanced Micro Devices, Inc.";
        case 0x8086:      return "Intel Corporation";
        case 0x102B:      return "Matrox Electronic Systems Ltd.";
        case 0x1414:      return "Microsoft Corporation";
        case 0x10DE:      return "NVIDIA Corporation";
        case 0x108E:      return "Oracle Corporation";
        case 0x15AD:      return "VMware Inc.";
        default:          return "";
    }
}

const char* DXFeatureLevelToVersion(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:    return "11.1";
        case D3D_FEATURE_LEVEL_11_0:    return "11.0";
        case D3D_FEATURE_LEVEL_10_1:    return "10.1";
        case D3D_FEATURE_LEVEL_10_0:    return "10.0";
        case D3D_FEATURE_LEVEL_9_3:     return "9.3";
        case D3D_FEATURE_LEVEL_9_2:     return "9.2";
        case D3D_FEATURE_LEVEL_9_1:     return "9.1";
    }
    return "";
}

bool InitializeDXInternal(D3D11Context *ctx, int width, int height)
{
    HRESULT hr = S_OK;
    UINT deviceFlags = 0;
    IDXGIDevice *pDXGIDevice = NULL;
    IDXGIAdapter *pAdapter = NULL;
    IDXGIFactory2 *pDXGIFactory = NULL;
    static const D3D_DRIVER_TYPE driverAttempts[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
    static const D3D_FEATURE_LEVEL levelAttempts[] =
        {
            // D3D_FEATURE_LEVEL_12_1,
            // D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1, // Direct3D 11.1 SM 6
            D3D_FEATURE_LEVEL_11_0, // Direct3D 11.0 SM 5
            D3D_FEATURE_LEVEL_10_1, // Direct3D 10.1 SM 4
            D3D_FEATURE_LEVEL_10_0, // Direct3D 10.0 SM 4
            D3D_FEATURE_LEVEL_9_3,  // Direct3D 9.3  SM 3
            D3D_FEATURE_LEVEL_9_2,  // Direct3D 9.2  SM 2
            D3D_FEATURE_LEVEL_9_1,  // Direct3D 9.1  SM 2
        };

    for (int driver = 0; driver < ARRAYSIZE(driverAttempts); driver++)
    {
        hr = D3D11CreateDevice(
            NULL,
            driverAttempts[driver],
            NULL,
            deviceFlags,
            levelAttempts,
            ARRAYSIZE(levelAttempts),
            D3D11_SDK_VERSION,
            &ctx->device,
            &ctx->featureLevel,
            &ctx->deviceContext);

        if (SUCCEEDED(hr))
        {
            // printf("feature level: 0x%X\n", this->featureLevel);
            break;
        }
    }
    if (SUCCEEDED(hr))
    {
        // __uuidof(IDXGIDevice);
        hr = D3D_API(ctx->device, QueryInterface, &IID_IDXGIDevice, (void **)(&pDXGIDevice));
    }
    if (SUCCEEDED(hr))
    {
        hr = D3D_API(pDXGIDevice, GetAdapter, &pAdapter);
    }
    if (SUCCEEDED(hr))
    {
        hr = D3D_API(pAdapter, GetParent, &IID_IDXGIFactory2, (void **)&pDXGIFactory);
    }
    // #if defined(__ALLOW_TEARING__)
    //         IDXGIFactory6* factory6;
    //         if (SUCCEEDED(hr))
    //         {
    //             hr = pAdapter->GetParent(__uuidof(IDXGIFactory6), (void**)&factory6);
    //         }
    //         if (SUCCEEDED(hr)) {
    //             BOOL allowTearing = FALSE;
    //             factory6->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
    //             this->tearingSupport = allowTearing == TRUE;
    //         }
    // #endif
    if (SUCCEEDED(hr))
    {
        DXGI_SWAP_CHAIN_DESC1 swapDesc;
        memset(&swapDesc, 0, sizeof(swapDesc));
        memset(&ctx->sampleDesc, 0, sizeof(ctx->sampleDesc));
        ctx->sampleDesc.Count = 1;
        ctx->sampleDesc.Quality = 0;
        swapDesc.SampleDesc.Count = ctx->sampleDesc.Count;
        swapDesc.SampleDesc.Quality = ctx->sampleDesc.Quality;
        swapDesc.Width = width;
        swapDesc.Height = height;
        swapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapDesc.Stereo = FALSE;
        swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapDesc.BufferCount = 2;
        swapDesc.Flags = 0;
        swapDesc.Scaling = DXGI_SCALING_STRETCH;
        swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
#ifdef __WINRT__
        if (IsWindows8OrGreater())
        {
            swapDesc.Scaling = DXGI_SCALING_NONE;
        }
        else
        {
            swapDesc.Scaling = DXGI_SCALING_STRETCH;
        }
#endif
#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
        swapDesc.Scaling = DXGI_SCALING_STRETCH;
        swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
#endif
        // if (this->tearingSupport) {
        //     swapDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
        // }
        // this->swapDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
#ifdef __WINRT__
        hr = D3D_API(pDXGIFactory, CreateSwapChainForCoreWindow,
                     (IUnknown *)ctx->device,
                     ctx->coreWindow,
                     &swapDesc,
                     NULL,
                     &ctx->swapChain);
#else
        hr = D3D_API(pDXGIFactory, CreateSwapChainForHwnd,
                     (IUnknown *)ctx->device,
                     ctx->hwnd,
                     &swapDesc,
                     NULL,
                     NULL,
                     &ctx->swapChain);
#endif
    }
    DXGI_ADAPTER_DESC desc;
    D3D_API(pAdapter, GetDesc, &desc);
    D3D_API_RELEASE(pDXGIDevice);
    D3D_API_RELEASE(pAdapter);
    D3D_API_RELEASE(pDXGIFactory);
    if (!SUCCEEDED(hr))
    {
        // Fail
        DestroyD3D11Context(ctx);
        return false;
    }

    char nameBuffer[256] = {0};
    wcstombs(nameBuffer, desc.Description, 256);
    strcat(ctx->renderInfo.deviceName, nameBuffer);
    sprintf(nameBuffer, "Direct3D %s", DXFeatureLevelToVersion(ctx->featureLevel));
    strcat(ctx->renderInfo.rendererName, nameBuffer);
    strcat(ctx->renderInfo.shadingLanguageName, "HLSL 5.0");
    strcat(ctx->renderInfo.vendorName, GetVendorName(desc.VendorId));
    return true;
}

D3D11Context *CreateD3D11Context(void *window, int width, int height)
{
    D3D11Context *ctx = (D3D11Context *)calloc(1, sizeof(D3D11Context));
#ifdef __WINRT__
    ctx->coreWindow = (IUnknown *)window;
#else
    ctx->hwnd = (HWND)window;
#endif
    if (!InitializeDXInternal(ctx, width, height))
    {
        return NULL;
    }
    _ResizeD3D11Drawable(ctx, width, height, true);
    return ctx;
}

void DestroyD3D11Context(D3D11Context *ctx)
{
    if (ctx->deviceContext)
    {
        ID3D11RenderTargetView *viewList[1] = {NULL};
        D3D_API(ctx->deviceContext, OMSetRenderTargets, 1, viewList, NULL);
    }
    D3D_API_RELEASE(ctx->deviceContext);
    D3D_API_RELEASE(ctx->device);
    D3D_API_RELEASE(ctx->swapChain);
    D3D_API_RELEASE(ctx->renderTargetView);
    D3D_API_RELEASE(ctx->depthStencil);
    D3D_API_RELEASE(ctx->depthStencilView);
    free(ctx);
    ctx = NULL;
}

bool ResizeD3D11Drawable(D3D11Context *ctx, int width, int height) {
    return _ResizeD3D11Drawable(ctx, width, height, false);
}
bool _ResizeD3D11Drawable(D3D11Context *ctx, int width, int height, bool init)
{
    D3D11_RENDER_TARGET_VIEW_DESC renderDesc;
    ID3D11RenderTargetView *viewList[1] = {NULL};
    HRESULT hr = S_OK;

    ID3D11Resource *pBackBufferResource = NULL;
    D3D11_TEXTURE2D_DESC texDesc;
    D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
    D3D_API(ctx->deviceContext, OMSetRenderTargets, 1, viewList, NULL);

    D3D_API_RELEASE(ctx->renderTargetView);
    D3D_API_RELEASE(ctx->depthStencilView);

    if (!init) {
        UINT resizeBufferFlags = 0;
        // if (ctx->tearingSupport) {
        //     resizeBufferFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
        // }
        hr = D3D_API(ctx->swapChain, ResizeBuffers, 2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, resizeBufferFlags);
        if (FAILED(hr))
        {
            return false;
        }
    }

    hr = D3D_API(ctx->swapChain, GetBuffer, 0, &IID_ID3D11Texture2D, (void **)&pBackBufferResource);
    if (FAILED(hr))
    {
        return false;
    }
    renderDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    renderDesc.ViewDimension = (ctx->sampleDesc.Count > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
    renderDesc.Texture2D.MipSlice = 0;
    hr = D3D_API(ctx->device, CreateRenderTargetView,
                 pBackBufferResource,
                 &renderDesc,
                 &ctx->renderTargetView);
    D3D_API_RELEASE(pBackBufferResource);
    if (FAILED(hr))
    {
        return false;
    }
    texDesc.ArraySize = 1;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    texDesc.CPUAccessFlags = 0;
    texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    texDesc.Height = (UINT)height;
    texDesc.Width = (UINT)width;
    texDesc.MipLevels = 1;
    texDesc.MiscFlags = 0;
    texDesc.SampleDesc.Count = ctx->sampleDesc.Count;
    texDesc.SampleDesc.Quality = ctx->sampleDesc.Quality;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    D3D_API_RELEASE(ctx->depthStencil);
    hr = D3D_API(ctx->device, CreateTexture2D,
                 &texDesc,
                 NULL,
                 &ctx->depthStencil);
    if (FAILED(hr))
    {
        return false;
    }
    depthViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthViewDesc.ViewDimension = (ctx->sampleDesc.Count > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
    depthViewDesc.Flags = 0;
    depthViewDesc.Texture2D.MipSlice = 0;
    hr = D3D_API(ctx->device, CreateDepthStencilView,
                 (ID3D11Resource *)ctx->depthStencil,
                 &depthViewDesc,
                 &ctx->depthStencilView);
    if (FAILED(hr))
    {
        return false;
    }
    D3D11_VIEWPORT viewport;
    viewport.Width = (float)width;
    viewport.Height = (float)height;
    viewport.MaxDepth = 1.0f;
    viewport.MinDepth = 0.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    D3D_API(ctx->deviceContext, RSSetViewports, 1, &viewport);
    return true;
}
float GetD3D11ScaleFactor(D3D11Context *ctx)
{
    float dpi = 1.0f;
#ifdef __WINRT__
    winrt::Windows::Graphics::Display::DisplayInformation displayInformation = winrt::Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
    dpi = (unsigned int)displayInformation.LogicalDpi() / 96.0f;
#else
    dpi = GetDpiForWindow(ctx->hwnd) / 96.0f;
#endif
    return dpi;
}

void ClearD3D11WithColor(D3D11Context *ctx, float clearColor[4])
{
    D3D_API(ctx->deviceContext, OMSetRenderTargets, 1, &ctx->renderTargetView, ctx->depthStencilView);
    D3D_API(ctx->deviceContext, ClearRenderTargetView,
            ctx->renderTargetView,
            clearColor);
    D3D_API(ctx->deviceContext, ClearDepthStencilView,
            ctx->depthStencilView,
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
            0.0f,
            (UINT8)0);
}

void D3D11Present(D3D11Context *ctx, int syncInterval)
{
    DXGI_PRESENT_PARAMETERS presentParameters;
    memset(&presentParameters, 0, sizeof(DXGI_PRESENT_PARAMETERS));
    D3D_API(ctx->swapChain, Present1, syncInterval, 0, &presentParameters);
}

NVGrendererInfo D3D11GetRenderInfo(D3D11Context *ctx)
{
    return ctx->renderInfo;
}

ID3D11Texture2D* D3D11GetSwapChainTexture(D3D11Context *ctx) {
    ID3D11Texture2D *tex;
	HRESULT hr;
    hr = D3D_API(ctx->swapChain, GetBuffer, 0, &IID_ID3D11Texture2D, (void**)&tex);
    if (!SUCCEEDED(hr)) {
        return NULL;
    }
    return tex;
}
