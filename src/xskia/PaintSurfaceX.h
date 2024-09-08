#pragma once

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "graph2d/PaintSurface.h"
#include "scene2d/geom_types.h"
#include "api/kwui/Application.h"
#ifdef _WIN32
#include "windows/windows_header.h"
#endif
#ifdef __ANDROID__
#include "android/native_window.h"
#include "android/SurfaceAndroid.h"
#endif
#include <tools/sk_app/WindowContext.h>

namespace xskia {

class PaintSurfaceX : public graph2d::PaintSurfaceInterface {
public:
    struct CreateInfo {
        kwui::PaintSurfaceType surface_type = kwui::PAINT_SURFACE_DEFAULT;
#ifdef _WIN32
        HWND hwnd = nullptr;
#endif
#ifdef __ANDROID__
        ANativeWindow* hwnd = nullptr;
#endif
        scene2d::DimensionF pixel_size;
        float dpi_scale = 1.0f;
    };
    static std::unique_ptr<PaintSurfaceX> create(const CreateInfo& config);
    void resize(int pixel_width, int pixel_height, float dpi_scale) override;
	std::unique_ptr<graph2d::PaintContextInterface> beginPaint() override;
	bool endPaint() override;
    void swapBuffers() override;
    void discardDeviceResources() override;

    sk_sp<SkImage> getCachedBitmap(const std::string& key) const;
    void updateCachedBitmap(const std::string& key, sk_sp<SkImage> bitmap);

private:
    PaintSurfaceX(const CreateInfo& config);
    void realizeSurfaceType();
    void createSurface();

    CreateInfo config_;
    std::unique_ptr<uint8_t[]> raster_buffer_; // PAINT_SURFACE_X_RASTER
    sk_sp<SkSurface> surface_;
    std::unique_ptr<sk_app::WindowContext> wnd_context_;
    std::unordered_map<std::string, sk_sp<SkImage>> bitmap_cache_;
#ifdef __ANDROID__
    std::unique_ptr<android::WindowSurface> wnd_surface_;
#endif
};
}