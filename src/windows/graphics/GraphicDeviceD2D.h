#pragma once

#include "windows/windows_header.h"
#include "TextLayoutD2D.h"
#include "TextFlowD2D.h"
#include "scene2d/geom_types.h"
#include "style/StyleFont.h"
#include "absl/types/span.h"
#include <unordered_map>

namespace windows::graphics
{
struct BitmapSubItem
{
    ComPtr<IWICBitmapFrameDecode> frame;
    ComPtr<IWICFormatConverter> converter;
    scene2d::PointF dpi_scale;

    operator bool() const
    {
        return frame != nullptr;
    }
};

struct BitmapItem
{
    BitmapSubItem x1;
    BitmapSubItem x1_5;
    BitmapSubItem x2;

    operator bool() const
    {
        return (bool)x1;
    }
};

struct WicBitmapRenderTarget
{
    ComPtr<IWICBitmap> bitmap;
    ComPtr<ID2D1BitmapRenderTarget> target;
    ComPtr<ID2D1GdiInteropRenderTarget> interop_target;

    operator bool() const
    {
        return (bool)target;
    }
};

struct HwndRenderTarget
{
    ComPtr<ID2D1DeviceContext> d2d1_ctx;
    ComPtr<IDXGISwapChain1> d2d1_swap_chain;
    ComPtr<ID2D1Bitmap1> d2d1_back_buffer;
    ComPtr<ID2D1HwndRenderTarget> hwnd_rt; // Direct2D 1.0 fallback

    explicit operator bool() const
    {
        return (hwnd_rt || d2d1_swap_chain);
    }
};

class GraphicDeviceD2D
{
public:
    ~GraphicDeviceD2D();
    static GraphicDeviceD2D* createInstance();
    static void releaseInstance();
    static GraphicDeviceD2D* instance();
    bool init();
    HwndRenderTarget createHwndRenderTarget(
        HWND hwnd, const scene2d::DimensionF& size, float dpi_scale);
    // Alpha premultiplied
    WicBitmapRenderTarget createWicBitmapRenderTarget(DXGI_FORMAT format,
                                                      float width, float height, float dpi_scale);
    ComPtr<ID2D1PathGeometry> createPathGeometry();
    ComPtr<ID2D1EllipseGeometry> createEllipseGeometry(float center_x, float center_y, float radius_x, float radius_y);
    ComPtr<ID2D1RectangleGeometry> createRectangleGeometry(const scene2d::RectF& rect);
    ComPtr<IDWriteTextLayout> createTextLayout(
        const std::wstring& text,
        const std::string& font_family,
        float font_size,
        style::FontWeight font_weight = style::FontWeight(),
        style::FontStyle font_style = style::FontStyle::Normal);
    std::unique_ptr<TextFlowD2D> createTextFlow(
        const std::wstring& text,
        float line_height,
        const std::string& font_family,
        float font_size,
        style::FontWeight font_weight = style::FontWeight(),
        style::FontStyle font_style = style::FontStyle::Normal);
    void updateTextFlow(
        TextFlowD2D* text_flow,
        const std::wstring& text,
        float line_height,
        const std::string& font_family,
        float font_size,
        style::FontWeight font_weight = style::FontWeight(),
        style::FontStyle font_style = style::FontStyle::Normal);
    std::string getDefaultFontFamily() const;
    bool getFontMetrics(const std::string& font_family, DWRITE_FONT_METRICS& out_metrics);
    void loadBitmapToCache(const std::string& name, absl::Span<const uint8_t> res_x1);
    void loadBitmapToCache(const std::string& name, absl::Span<const uint8_t> res_x1,
                           absl::Span<const uint8_t> res_x1_5, absl::Span<const uint8_t> res_x2);
    void loadBitmapToCache(const std::string& name, const std::wstring& filename);
    BitmapSubItem getBitmap(const std::string& name, float dpi_scale = 1.0f);
    float getInitialDesktopDpiScale() const;
    void addFont(const char* family_name, const uint8_t* data, size_t size);
    // Map font and style to fontFace.
    ComPtr<IDWriteFontFace> getFirstMatchingFontFace(
        const wchar_t* family_name,
        DWRITE_FONT_WEIGHT weight,
        DWRITE_FONT_STRETCH stretch,
        DWRITE_FONT_STYLE style);

    ID3D11Device1* getD3DDevice1() const { return d2d1_.dev3d.Get(); }
    HRESULT createNativeBitmap(float width, float height, ComPtr<ID3D11Texture2D>& out_tex,
                               ComPtr<ID2D1Bitmap1>& out_bitmap);
    ComPtr<ID2D1Bitmap1> createBitmap(float width, float height);

private:
    BitmapSubItem loadBitmapFromResource(absl::Span<const uint8_t> res, const scene2d::PointF& dpi_scale);
    BitmapSubItem loadBitmapFromFilename(const std::wstring& filename, const scene2d::PointF& dpi_scale);
    BitmapSubItem loadBitmapFromStream(ComPtr<IWICStream> stream, const scene2d::PointF& dpi_scale);
    void loadBitmapToCache(const std::string& name);

    bool use_d2d1_ = false;

    struct
    {
        ComPtr<ID2D1Factory> factory;
    } d2d0_;

    struct
    {
        D3D_FEATURE_LEVEL feature_level;
        ComPtr<IDXGIDevice> devdx;
        ComPtr<ID3D11Device1> dev3d;
        ComPtr<ID3D11DeviceContext1> ctx3d;
        ComPtr<ID2D1Device> dev2d;
        ComPtr<ID2D1DeviceContext> ctx2d;
        ComPtr<ID2D1Factory1> factory;
    } d2d1_;

    ComPtr<IDWriteFactory> dwrite_;
    ComPtr<IDWriteFontCollection> font_collection_;
    ComPtr<IWICImagingFactory> wic_factory_;
    std::unordered_map<std::string, BitmapItem> bitmap_cache_;
    std::unordered_map<std::wstring, ComPtr<IDWriteFontFace>> font_cache_;
};
} // namespace windows::graphics
