#include "GraphicDeviceD2D.h"
#include "graph2d/PaintSurface.h"
#include "scene2d/geom_types.h"
#include "windows/windows_header.h"

namespace windows::graphics
{

class PaintSurfaceWindowD2D : public graph2d::PaintSurfaceInterface {
public:
    struct Configuration {
        HWND hwnd = NULL;
        scene2d::DimensionF size;
        float dpi_scale = 1.0f;
    };

    static std::unique_ptr<PaintSurfaceWindowD2D> create(const Configuration& config);
    void resize(int pixel_width, int pixel_height, float dpi_scale) override;
    std::unique_ptr<graph2d::PaintContextInterface> beginPaint() override;
    bool endPaint() override;
    void swapBuffers() override;

private:
    PaintSurfaceWindowD2D(const Configuration& config);
    void recreateRenderTarget();

    Configuration config_;
    HwndRenderTarget hwnd_rt_;
};

class PaintSurfaceBitmapD2D : public graph2d::PaintSurfaceInterface {
public:
    struct Configuration {
        DXGI_FORMAT format;
        scene2d::DimensionF pixel_size;
        float dpi_scale = 1.0f;
    };

    static std::unique_ptr<PaintSurfaceBitmapD2D> create(const Configuration& config);
    void resize(int pixel_width, int pixel_height, float dpi_scale) override;
    std::unique_ptr<graph2d::PaintContextInterface> beginPaint() override;
    bool endPaint() override;
    void swapBuffers() override;

    ComPtr<IWICBitmapLock> map();
    IWICBitmap* getWicBitmap() const;

private:
    PaintSurfaceBitmapD2D(const Configuration& config);
    void recreateRenderTarget();

    Configuration config_;
    WicBitmapRenderTarget wic_rt_;
};

} // namespace windows::graphics
