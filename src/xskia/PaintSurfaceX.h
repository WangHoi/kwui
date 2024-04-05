#pragma once

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "graph2d/PaintSurface.h"
#include "scene2d/geom_types.h"
#ifdef _WIN32
#include "windows/windows_header.h"
#endif

namespace xskia {
class PaintSurfaceX : public graph2d::PaintSurfaceInterface {
public:
    struct Configuration {
#ifdef _WIN32
        HWND hwnd;
#endif
        scene2d::DimensionF size;
        float dpi_scale = 1.0f;
    };
    static std::unique_ptr<PaintSurfaceX> create(const Configuration& config);
    void resize(int pixel_width, int pixel_height, float dpi_scale) override;
	std::unique_ptr<graph2d::PainterInterface> beginPaint() override;
	bool endPaint() override;

private:
    PaintSurfaceX(const Configuration& config);
    void createSurface();

    Configuration config_;
    std::unique_ptr<uint8_t[]> buffer_;
    sk_sp<SkSurface> surface_;
};
}