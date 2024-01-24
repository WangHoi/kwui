#pragma once

#include "windows/windows_header.h"
#include "TextLayout.h"
#include "TextFlow.h"
#include "scene2d/geom_types.h"
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
	static GraphicDevice* createInstance();
	static void releaseInstance();
	static GraphicDevice* instance();
	bool Init();
	ComPtr<ID2D1HwndRenderTarget> CreateHwndRenderTarget(
		HWND hwnd, const scene2d::DimensionF& size, float dpi_scale);
	WicBitmapRenderTarget CreateWicBitmapRenderTarget(
		float width, float height, float dpi_scale);
	ComPtr<IDWriteTextLayout> CreateTextLayout(
		const std::wstring& text,
		const std::string& font_family,
		float font_size,
		FontWeight font_weight = FontWeight(),
		FontStyle font_style = FontStyle());
	std::unique_ptr<TextFlow> CreateTextFlow(
		const std::wstring& text,
		float line_height,
		const std::string& font_family,
		float font_size,
		FontWeight font_weight = FontWeight(),
		FontStyle font_style = FontStyle());
	std::string GetDefaultFontFamily() const;
	bool GetFontMetrics(const std::string& font_family, DWRITE_FONT_METRICS& out_metrics);
	void LoadBitmapToCache(const std::string& name, absl::Span<const uint8_t> res_x1);
	void LoadBitmapToCache(const std::string& name, absl::Span<const uint8_t> res_x1,
		absl::Span<const uint8_t> res_x1_5, absl::Span<const uint8_t> res_x2);
	void LoadBitmapToCache(const std::string& name, const std::wstring& filename);
	BitmapSubItem GetBitmap(const std::string& name, float dpi_scale = 1.0f);
	float GetInitialDesktopDpiScale() const;

private:
	BitmapSubItem LoadBitmapFromResource(absl::Span<const uint8_t> res, const scene2d::PointF& dpi_scale);
	BitmapSubItem LoadBitmapFromFilename(const std::wstring& filename, const scene2d::PointF& dpi_scale);
	BitmapSubItem LoadBitmapFromStream(ComPtr<IWICStream> stream, const scene2d::PointF& dpi_scale);
	void LoadBitmapToCache(const std::string& name);

	ComPtr<ID2D1Factory> _factory;
	ComPtr<IDWriteFactory> _dwrite;
	ComPtr<IDWriteFontCollection> _font_collection;
	ComPtr<IWICImagingFactory> _wic_factory;
	std::unordered_map<std::string, BitmapItem> _bitmap_cache;
};

} // namespace graphics
} // namespace windows
