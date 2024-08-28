#include "PaintSurfaceD2D.h"
#include "GraphicDeviceD2D.h"
#include "PainterD2D.h"

namespace windows::graphics
{
std::unique_ptr<PaintSurfaceWindowD2D> PaintSurfaceWindowD2D::create(const Configuration& config)
{
    return std::unique_ptr<PaintSurfaceWindowD2D>(new PaintSurfaceWindowD2D(config));
}

void PaintSurfaceWindowD2D::discardDeviceResources()
{
    bitmap_cache_.clear();
}

void PaintSurfaceWindowD2D::resize(int pixel_width, int pixel_height, float dpi_scale)
{
    config_.size.width = pixel_width;
    config_.size.height = pixel_height;
    config_.dpi_scale = dpi_scale;
    if (!hwnd_rt_) {
        recreateRenderTarget();
        return;
    }
    HRESULT hr;
    if (hwnd_rt_.d2d1_ctx) {
        do {
            hwnd_rt_.d2d1_back_buffer = nullptr;
            hr = hwnd_rt_.d2d1_swap_chain->ResizeBuffers(2, (UINT32)pixel_width, (UINT32)pixel_height,
                                                         DXGI_FORMAT_B8G8R8A8_UNORM, 0);
            if (FAILED(hr)) break;

            // Get SwapChain BackBuffer
            ComPtr<IDXGISurface> dxgi_back_buffer;
            hr = hwnd_rt_.d2d1_swap_chain->GetBuffer(0, IID_PPV_ARGS(&dxgi_back_buffer));
            if (FAILED(hr)) break;

            // Create a Direct2D surface (bitmap) linked to the Direct3D texture back buffer via the DXGI back buffer
            const FLOAT dpi = dpi_scale * USER_DEFAULT_SCREEN_DPI;
            D2D1_BITMAP_PROPERTIES1 bitmapProperties =
                D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                                        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), dpi,
                                        dpi);
            hr = hwnd_rt_.d2d1_ctx->CreateBitmapFromDxgiSurface(dxgi_back_buffer.Get(), &bitmapProperties,
                                                                hwnd_rt_.d2d1_back_buffer.GetAddressOf());
        } while (false);
    } else if (hwnd_rt_.hwnd_rt) {
        hr = hwnd_rt_.hwnd_rt->Resize(D2D1::SizeU((UINT32)pixel_width, (UINT32)pixel_height));
        hwnd_rt_.hwnd_rt->SetDpi(dpi_scale * USER_DEFAULT_SCREEN_DPI, dpi_scale * USER_DEFAULT_SCREEN_DPI);
    }
    //LOG(INFO) << "RT resize " << (UINT32)_pixel_size.width << "x" << (UINT32)_pixel_size.height << "px";
    if (hr == D2DERR_RECREATE_TARGET)
        recreateRenderTarget();
}

std::unique_ptr<graph2d::PaintContextInterface> PaintSurfaceWindowD2D::beginPaint()
{
    if (!hwnd_rt_)
        recreateRenderTarget();
    if (!hwnd_rt_)
        return nullptr;
    ID2D1RenderTarget* d2d_rt;
    if (hwnd_rt_.d2d1_ctx) {
        hwnd_rt_.d2d1_ctx->SetTarget(hwnd_rt_.d2d1_back_buffer.Get());
        const float dpi = config_.dpi_scale * USER_DEFAULT_SCREEN_DPI;
        hwnd_rt_.d2d1_ctx->SetDpi(dpi, dpi);
        hwnd_rt_.d2d1_ctx->BeginDraw();
        d2d_rt = hwnd_rt_.d2d1_ctx.Get();
    } else {
        hwnd_rt_.hwnd_rt->BeginDraw();
        d2d_rt = hwnd_rt_.hwnd_rt.Get();
    }
    return std::make_unique<PainterImpl>(this, d2d_rt, scene2d::PointF());
}

bool PaintSurfaceWindowD2D::endPaint()
{
    if (!hwnd_rt_) return false;

    HRESULT hr;
    if (hwnd_rt_.d2d1_ctx) {
        hr = hwnd_rt_.d2d1_ctx->EndDraw();
        hwnd_rt_.d2d1_ctx->SetTarget(nullptr);
    } else {
        hr = hwnd_rt_.hwnd_rt->EndDraw();
    }
    if (hr != S_OK) {
        hwnd_rt_ = HwndRenderTarget();
        return false;
    }
    return true;
}

void PaintSurfaceWindowD2D::swapBuffers()
{
    if (hwnd_rt_.d2d1_swap_chain) {
        DXGI_PRESENT_PARAMETERS parameters = {};
        parameters.DirtyRectsCount = 0;
        parameters.pDirtyRects = nullptr;
        parameters.pScrollRect = nullptr;
        parameters.pScrollOffset = nullptr;

        (void)hwnd_rt_.d2d1_swap_chain->Present1(1, 0, &parameters);
    }
}

std::shared_ptr<BitmapD2D> PaintSurfaceWindowD2D::getCachedBitmap(const std::string& key) const
{
    const auto it = bitmap_cache_.find(key);
    return (it == bitmap_cache_.end()) ? nullptr : it->second;
}

void PaintSurfaceWindowD2D::updateCachedBitmap(const std::string& key, std::shared_ptr<BitmapD2D> bitmap)
{
    bitmap_cache_[key] = bitmap;
}

PaintSurfaceWindowD2D::PaintSurfaceWindowD2D(const Configuration& config)
    : config_(config)
{
    recreateRenderTarget();
}

void PaintSurfaceWindowD2D::recreateRenderTarget()
{
    // Release resources first
    hwnd_rt_ = HwndRenderTarget();

    hwnd_rt_ = GraphicDeviceD2D::instance()->createHwndRenderTarget(
        config_.hwnd, config_.size, config_.dpi_scale);
    LOG(INFO) << "PaintSurfaceWindowD2D::recreateRenderTarget() hwnd=" << std::hex << config_.hwnd
        << " size=" << config_.size
        << " dpi_scale=" << config_.dpi_scale;
}

////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<PaintSurfaceBitmapD2D> PaintSurfaceBitmapD2D::create(const Configuration& config)
{
    return std::unique_ptr<PaintSurfaceBitmapD2D>(new PaintSurfaceBitmapD2D(config));
}

void PaintSurfaceBitmapD2D::resize(int pixel_width, int pixel_height, float dpi_scale)
{
    config_.pixel_size.width = pixel_width;
    config_.pixel_size.height = pixel_height;
    config_.dpi_scale = dpi_scale;
    recreateRenderTarget();
}

std::unique_ptr<graph2d::PaintContextInterface> PaintSurfaceBitmapD2D::beginPaint()
{
    if (!wic_rt_)
        recreateRenderTarget();
    if (!wic_rt_)
        return nullptr;
    wic_rt_.target->BeginDraw();
    return std::make_unique<PainterImpl>(nullptr, wic_rt_.target.Get(), scene2d::PointF());
}

bool PaintSurfaceBitmapD2D::endPaint()
{
    if (!wic_rt_) return false;

    HRESULT hr;
    if (wic_rt_.target) {
        hr = wic_rt_.target->EndDraw();
    }
    if (hr != S_OK) {
        wic_rt_ = WicBitmapRenderTarget();
        return false;
    }
    return true;
}

void PaintSurfaceBitmapD2D::swapBuffers()
{
}

ComPtr<IWICBitmapLock> PaintSurfaceBitmapD2D::map()
{
    if (wic_rt_) {
        WICRect rc{0, 0, (INT)config_.pixel_size.width, (INT)config_.pixel_size.height};
        ComPtr<IWICBitmapLock> lock;
        wic_rt_.bitmap->Lock(&rc, WICBitmapLockRead | WICBitmapLockWrite, lock.GetAddressOf());
        return lock;
    }
    return nullptr;
}

IWICBitmap* PaintSurfaceBitmapD2D::getWicBitmap() const
{
    if (wic_rt_.bitmap)
        return wic_rt_.bitmap.Get();

    return nullptr;
}

PaintSurfaceBitmapD2D::PaintSurfaceBitmapD2D(const Configuration& config)
    : config_(config)
{
    recreateRenderTarget();
}

void PaintSurfaceBitmapD2D::recreateRenderTarget()
{
    // Release resources first
    wic_rt_ = WicBitmapRenderTarget();

    wic_rt_ = GraphicDeviceD2D::instance()
        ->createWicBitmapRenderTarget(config_.format,
                                      config_.pixel_size.width / config_.dpi_scale,
                                      config_.pixel_size.height / config_.dpi_scale,
                                      config_.dpi_scale);
    LOG(INFO) << "PaintSurfaceBitmapD2D::recreateRenderTarget() hwnd=" << std::hex << config_.format
        << " pixel_size=" << config_.pixel_size
        << " dpi_scale=" << config_.dpi_scale;
}
} // namespace windows::graphics
