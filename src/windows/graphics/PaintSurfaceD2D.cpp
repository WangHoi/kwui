#include "PaintSurfaceD2D.h"
#include "GraphicDeviceD2D.h"
#include "PainterD2D.h"

namespace windows::graphics
{

std::unique_ptr<PaintSurfaceD2D> PaintSurfaceD2D::create(const Configuration& config)
{
    return std::unique_ptr<PaintSurfaceD2D>(new PaintSurfaceD2D(config));
}
void PaintSurfaceD2D::resize(int pixel_width, int pixel_height, float dpi_scale) {
    config_.size.width = pixel_width;
    config_.size.height = pixel_height;
    config_.dpi_scale = dpi_scale;
    if (!rt_) {
        recreateRenderTarget();
        return;
    }
    HRESULT hr;
    if (rt_.d2d1_ctx) {
        do {
            rt_.d2d1_back_buffer = nullptr;
            hr = rt_.d2d1_swap_chain->ResizeBuffers(2, (UINT32)pixel_width, (UINT32)pixel_height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
            if (FAILED(hr)) break;

            // Get SwapChain BackBuffer
            ComPtr<IDXGISurface> dxgi_back_buffer;
            hr = rt_.d2d1_swap_chain->GetBuffer(0, IID_PPV_ARGS(&dxgi_back_buffer));
            if (FAILED(hr)) break;

            // Create a Direct2D surface (bitmap) linked to the Direct3D texture back buffer via the DXGI back buffer
            const FLOAT dpi = dpi_scale * USER_DEFAULT_SCREEN_DPI;
            D2D1_BITMAP_PROPERTIES1 bitmapProperties =
                    D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                                            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), dpi,
                                            dpi);
            hr = rt_.d2d1_ctx->CreateBitmapFromDxgiSurface(dxgi_back_buffer.Get(), &bitmapProperties,
                                                          rt_.d2d1_back_buffer.GetAddressOf());
        } while (false);
    } else if (rt_.hwnd_rt) {
        hr = rt_.hwnd_rt->Resize(D2D1::SizeU((UINT32)pixel_width, (UINT32)pixel_height));
        rt_.hwnd_rt->SetDpi(dpi_scale * USER_DEFAULT_SCREEN_DPI, dpi_scale * USER_DEFAULT_SCREEN_DPI);
    }
    //LOG(INFO) << "RT resize " << (UINT32)_pixel_size.width << "x" << (UINT32)_pixel_size.height << "px";
    if (hr == D2DERR_RECREATE_TARGET)
        recreateRenderTarget();
}
std::unique_ptr<graph2d::PainterInterface> PaintSurfaceD2D::beginPaint() {
    if (!rt_)
        recreateRenderTarget();
    if (!rt_)
        return nullptr;
    ID2D1RenderTarget* d2d_rt;
    if (rt_.d2d1_ctx) {
        rt_.d2d1_ctx->SetTarget(rt_.d2d1_back_buffer.Get());
        const float dpi = config_.dpi_scale * USER_DEFAULT_SCREEN_DPI;
        rt_.d2d1_ctx->SetDpi(dpi, dpi);
        rt_.d2d1_ctx->BeginDraw();
        d2d_rt = rt_.d2d1_ctx.Get();
    } else {
        rt_.hwnd_rt->BeginDraw();
        d2d_rt = rt_.hwnd_rt.Get();
    }
    return std::make_unique<PainterImpl>(d2d_rt, scene2d::PointF());
}
bool PaintSurfaceD2D::endPaint() {
    if (!rt_) return false;

    HRESULT hr;
    if (rt_.d2d1_ctx) {
        hr = rt_.d2d1_ctx->EndDraw();
        rt_.d2d1_ctx->SetTarget(nullptr);
    } else {
        hr = rt_.hwnd_rt->EndDraw();
    }
    if (hr != S_OK) {
        rt_ = HwndRenderTarget();
        return false;
    }
    return true;
}
void PaintSurfaceD2D::swapBuffers() {
    if (rt_.d2d1_swap_chain) {
        DXGI_PRESENT_PARAMETERS parameters = {};
        parameters.DirtyRectsCount = 0;
        parameters.pDirtyRects = nullptr;
        parameters.pScrollRect = nullptr;
        parameters.pScrollOffset = nullptr;

        (void)rt_.d2d1_swap_chain->Present1(1, 0, &parameters);
    }
}
PaintSurfaceD2D::PaintSurfaceD2D(const Configuration& config)
    : config_(config) {
    recreateRenderTarget();
}
void PaintSurfaceD2D::recreateRenderTarget() {
    // Release resources first
    rt_ = HwndRenderTarget();

    rt_ = GraphicDeviceD2D::instance()->createHwndRenderTarget(
        config_.hwnd, config_.size, config_.dpi_scale);
    LOG(INFO) << "PaintSurfaceD2D::recreateRenderTarget() hwnd=" << std::hex << config_.hwnd
        << " size=" << config_.size
        << " dpi_scale=" << config_.dpi_scale;
}

} // namespace windows::graphics
