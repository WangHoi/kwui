#include "graph2d/PaintSurface.h"

namespace windows
{
namespace graphics
{

class PaintSurfaceD2D : public graph2d::PaintSurfaceInterface {
public:
    void resize(int pixel_width, int pixel_height, float dpi_scale) override;
    std::unique_ptr<graph2d::PainterInterface> beginPaint() override;
    bool endPaint() override;
};

} // namespace graphics

} // namespace windows
