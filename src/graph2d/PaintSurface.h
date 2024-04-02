#pragma once
#include <memory>

namespace graph2d
{
class PainterInterface;
class PaintSurfaceInterface {
public:
    virtual void resize(int pixel_width, int pixel_height, float dpi_scale) = 0;
    virtual std::unique_ptr<PainterInterface> beginPaint() = 0;
    virtual bool endPaint() = 0;
};

} // namespace graph2d
