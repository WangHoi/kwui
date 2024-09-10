#include "GraphicDeviceD2D.h"
#include "graph2d/PaintSurface.h"
#include "scene2d/geom_types.h"
#include "windows/windows_header.h"

namespace windows::graphics
{
class BitmapD2D;

class PaintSurfaceWindowD2D : public graph2d::PaintSurfaceInterface {
public:
    struct Configuration {
        kwui::PaintSurfaceType surface_type = kwui::PAINT_SURFACE_DEFAULT;
        HWND hwnd = NULL;
        scene2d::DimensionF size;
        float dpi_scale = 1.0f;
    };

    static std::unique_ptr<PaintSurfaceWindowD2D> create(const Configuration& config);
    void discardDeviceResources() override;
    void resize(int pixel_width, int pixel_height, float dpi_scale) override;
    std::unique_ptr<graph2d::PaintContextInterface> beginPaint() override;
    bool endPaint() override;
    void swapBuffers() override;

    std::shared_ptr<BitmapD2D> getCachedBitmap(const std::string& key) const;
    void updateCachedBitmap(const std::string& key, std::shared_ptr<BitmapD2D> bitmap);

private:
    PaintSurfaceWindowD2D(const Configuration& config);
    void realizeSurfaceType();
    void recreateRenderTarget();

    Configuration config_;
    HwndRenderTarget hwnd_rt_;
    std::unordered_map<std::string, std::shared_ptr<BitmapD2D>> bitmap_cache_;
};

class PaintSurfaceBitmapD2D : public graph2d::PaintSurfaceInterface {
public:
    struct CreateInfo {
        DXGI_FORMAT format;
        scene2d::DimensionF pixel_size;
        float dpi_scale = 1.0f;
    };

    static std::unique_ptr<PaintSurfaceBitmapD2D> create(const CreateInfo& config);
    void discardDeviceResources() override {}
    void resize(int pixel_width, int pixel_height, float dpi_scale) override;
    std::unique_ptr<graph2d::PaintContextInterface> beginPaint() override;
    bool endPaint() override;
    void swapBuffers() override;

    ComPtr<IWICBitmapLock> map();
    IWICBitmap* getWicBitmap() const;

private:
    PaintSurfaceBitmapD2D(const CreateInfo& config);
    void recreateRenderTarget();

    CreateInfo config_;
    WicBitmapRenderTarget wic_rt_;
};

} // namespace windows::graphics
