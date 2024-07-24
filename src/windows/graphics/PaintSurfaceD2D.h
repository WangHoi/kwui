#include "GraphicDeviceD2D.h"
#include "graph2d/PaintSurface.h"
#include "scene2d/geom_types.h"
#include "windows/windows_header.h"

namespace windows::graphics
{

class PaintSurfaceD2D : public graph2d::PaintSurfaceInterface {
public:
    struct Configuration {
        HWND hwnd = NULL;
        scene2d::DimensionF size;
        float dpi_scale = 1.0f;
    };

    static std::unique_ptr<PaintSurfaceD2D> create(const Configuration& config);
    void resize(int pixel_width, int pixel_height, float dpi_scale) override;
    std::unique_ptr<graph2d::PainterInterface> beginPaint() override;
    bool endPaint() override;
    void swapBuffers() override;

private:
    PaintSurfaceD2D(const Configuration& config);
    void recreateRenderTarget();

    Configuration config_;
    HwndRenderTarget rt_;
};

} // namespace windows::graphics
