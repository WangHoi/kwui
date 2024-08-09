#pragma once
#include <memory>

namespace graph2d
{
class PaintContextInterface;
class PaintSurfaceInterface {
public:
    virtual ~PaintSurfaceInterface() = default;
    virtual void resize(int pixel_width, int pixel_height, float dpi_scale) = 0;
    virtual std::unique_ptr<PaintContextInterface> beginPaint() = 0;
    virtual bool endPaint() = 0;
    virtual void swapBuffers() = 0;
};

} // namespace graph2d
