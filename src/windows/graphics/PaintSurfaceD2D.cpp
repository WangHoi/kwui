#include "PaintSurfaceD2D.h"
#include "GraphicDevice.h"
#include "Painter.h"

namespace windows
{
namespace graphics
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
    HRESULT hr = rt_->Resize(D2D1::SizeU((UINT32)pixel_width, (UINT32)pixel_height));
    //LOG(INFO) << "RT resize " << (UINT32)_pixel_size.width << "x" << (UINT32)_pixel_size.height << "px";
    rt_->SetDpi(dpi_scale * USER_DEFAULT_SCREEN_DPI, dpi_scale * USER_DEFAULT_SCREEN_DPI);
    if (hr == D2DERR_RECREATE_TARGET)
        recreateRenderTarget();
}
std::unique_ptr<graph2d::PainterInterface> PaintSurfaceD2D::beginPaint() {
    if (!rt_)
        recreateRenderTarget();
    if (!rt_)
        return nullptr;
    rt_->BeginDraw();
    return std::make_unique<PainterImpl>(rt_.Get(), scene2d::PointF());
}
bool PaintSurfaceD2D::endPaint() {
    HRESULT hr = rt_->EndDraw();
    if (hr != S_OK) {
        rt_ = nullptr;
        return false;
    }
    return true;
}
PaintSurfaceD2D::PaintSurfaceD2D(const Configuration& config)
    : config_(config) {
    recreateRenderTarget();
}
void PaintSurfaceD2D::recreateRenderTarget() {
    rt_ = GraphicDevice::instance()->createHwndRenderTarget(
        config_.hwnd, config_.size, config_.dpi_scale);
    LOG(INFO) << "PaintSurfaceD2D::recreateRenderTarget() hwnd=" << std::hex << config_.hwnd
        << " size=" << config_.size
        << " dpi_scale=" << config_.dpi_scale
        << " rt=" << std::hex << rt_.Get();
}

} // namespace graphics

} // namespace windows
