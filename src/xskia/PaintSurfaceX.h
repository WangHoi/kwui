#pragma once

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "graph2d/PaintSurface.h"
#include "scene2d/geom_types.h"
#ifdef _WIN32
#include "windows/windows_header.h"
#endif
#ifdef __ANDROID__
#include "android/native_window.h"
#include "android/SurfaceAndroid.h"
#endif

namespace xskia {
class PaintSurfaceX : public graph2d::PaintSurfaceInterface {
public:
    struct Configuration {
#ifdef _WIN32
        HWND hwnd;
#endif
#ifdef __ANDROID__
        ANativeWindow* hwnd;
#endif
        scene2d::DimensionF pixel_size;
        float dpi_scale = 1.0f;
    };
    static std::unique_ptr<PaintSurfaceX> create(const Configuration& config);
    void resize(int pixel_width, int pixel_height, float dpi_scale) override;
	std::unique_ptr<graph2d::PaintContextInterface> beginPaint() override;
	bool endPaint() override;
    void swapBuffers() override;
    void discardDeviceResources() override;

    sk_sp<SkImage> getCachedBitmap(const std::string& key) const;
    void updateCachedBitmap(const std::string& key, sk_sp<SkImage> bitmap);

private:
    PaintSurfaceX(const Configuration& config);
    void createSurface();

    Configuration config_;
    std::unique_ptr<uint8_t[]> buffer_;
    sk_sp<SkSurface> surface_;
    std::unordered_map<std::string, sk_sp<SkImage>> bitmap_cache_;
#ifdef __ANDROID__
    std::unique_ptr<android::WindowSurface> wnd_surface_;
#endif
};
}