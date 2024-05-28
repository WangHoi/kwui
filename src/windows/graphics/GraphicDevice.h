#pragma once

#include "windows/windows_header.h"
#include "TextLayoutD2D.h"
#include "TextFlowD2D.h"
#include "scene2d/geom_types.h"
#include "style/StyleFont.h"
#include "absl/types/span.h"
#include <unordered_map>

namespace windows {
namespace graphics {

struct BitmapSubItem {
	ComPtr<IWICBitmapFrameDecode> frame;
	ComPtr<IWICFormatConverter> converter;
	scene2d::PointF dpi_scale;
	operator bool() const {
		return frame != nullptr;
	}
};
struct BitmapItem {
	BitmapSubItem x1;
	BitmapSubItem x1_5;
	BitmapSubItem x2;
	operator bool() const {
		return (bool)x1;
	}
};

struct WicBitmapRenderTarget {
	ComPtr<IWICBitmap> bitmap;
	ComPtr<ID2D1BitmapRenderTarget> target;
	ComPtr<ID2D1GdiInteropRenderTarget> interop_target;
};
class GraphicDevice {
public:
	~GraphicDevice();
	static GraphicDevice* createInstance();
	static void releaseInstance();
	static GraphicDevice* instance();
	bool Init();
	ComPtr<ID2D1HwndRenderTarget> createHwndRenderTarget(
		HWND hwnd, const scene2d::DimensionF& size, float dpi_scale);
	WicBitmapRenderTarget createWicBitmapRenderTarget(
		float width, float height, float dpi_scale);
	ComPtr<ID2D1PathGeometry> createPathGeometry();
	ComPtr<ID2D1EllipseGeometry> createEllipseGeometry(float center_x, float center_y, float radius_x, float radius_y);
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
		DWRITE_FONT_WEIGHT  weight,
		DWRITE_FONT_STRETCH stretch,
		DWRITE_FONT_STYLE   style);

private:
	BitmapSubItem LoadBitmapFromResource(absl::Span<const uint8_t> res, const scene2d::PointF& dpi_scale);
	BitmapSubItem loadBitmapFromFilename(const std::wstring& filename, const scene2d::PointF& dpi_scale);
	BitmapSubItem loadBitmapFromStream(ComPtr<IWICStream> stream, const scene2d::PointF& dpi_scale);
	void loadBitmapToCache(const std::string& name);

	ComPtr<ID2D1Factory> factory_;
	ComPtr<IDWriteFactory> dwrite_;
	ComPtr<IDWriteFontCollection> font_collection_;
	ComPtr<IWICImagingFactory> wic_factory_;
	std::unordered_map<std::string, BitmapItem> bitmap_cache_;
	std::unordered_map<std::wstring, ComPtr<IDWriteFontFace>> font_cache_;
};

} // namespace graphics
} // namespace windows
