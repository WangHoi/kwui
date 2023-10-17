// #pragma warning(disable:4996)
#include "GraphicDevice.h"
#include "base/log.h"
#include "windows/EncodingManager.h"
#include <numeric>

namespace windows {
namespace graphics {

static const wchar_t* DEFAULT_LOCALE = L"zh-CN";
static GraphicDevice *gdev = nullptr;

GraphicDevice* GraphicDevice::createInstance()
{
	if (!gdev) {
		gdev = new GraphicDevice();
	}
	return gdev;
}

void GraphicDevice::releaseInstance()
{
	if (gdev) {
		delete gdev;
		gdev = nullptr;
	}
}

GraphicDevice* GraphicDevice::instance()
{
	return gdev;
}

bool GraphicDevice::Init() {
	HRESULT hr;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, _factory.GetAddressOf());
	if (FAILED(hr)) return false;

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(_dwrite.GetAddressOf()));
	if (FAILED(hr)) {
		LOG(ERROR) << "DWriteCreateFactory failed hr=" << std::hex << hr;
		return false;
	}

	hr = _dwrite->GetSystemFontCollection(_font_collection.GetAddressOf());
	if (FAILED(hr)) {
		LOG(ERROR) << "DWriteFactory::GetSystemFontCollection() failed hr=" << std::hex << hr;
		return false;
	}

	hr = CoCreateInstance(CLSID_WICImagingFactory1,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IWICImagingFactory,
		(LPVOID*)_wic_factory.GetAddressOf());
	if (FAILED(hr)) {
		LOG(ERROR) << "CoCreateInstance(CLSID_WICImagingFactory1) failed hr=" << std::hex << hr;
		return false;
	}

	return true;
}

ComPtr<ID2D1HwndRenderTarget> GraphicDevice::CreateHwndRenderTarget(HWND hwnd, const scene2d::DimensionF& size, float dpi_scale) {
	ComPtr<ID2D1HwndRenderTarget> rt;
	D2D1_RENDER_TARGET_PROPERTIES rt_props
		= D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(),
			dpi_scale * USER_DEFAULT_SCREEN_DPI,
			dpi_scale * USER_DEFAULT_SCREEN_DPI);
	scene2d::DimensionF pixel_size = (size * dpi_scale).makeRound();
	D2D1_HWND_RENDER_TARGET_PROPERTIES hwnd_rt_props
		= D2D1::HwndRenderTargetProperties(
			hwnd,
			D2D1::SizeU((UINT)pixel_size.width, (UINT)pixel_size.height),
			D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS | D2D1_PRESENT_OPTIONS_IMMEDIATELY);
	HRESULT hr = _factory->CreateHwndRenderTarget(
		rt_props,
		hwnd_rt_props,
		rt.GetAddressOf());
	return rt;
}

WicBitmapRenderTarget GraphicDevice::CreateWicBitmapRenderTarget(
	float width, float height, float dpi_scale) {
	scene2d::DimensionF pixel_size = (scene2d::DimensionF(width, height) * dpi_scale).makeRound();
	WicBitmapRenderTarget rt;
	HRESULT hr;
	hr = _wic_factory->CreateBitmap((UINT)pixel_size.width,
		(UINT)pixel_size.height,
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapCacheOnLoad,
		rt.bitmap.GetAddressOf());
	if (!SUCCEEDED(hr))
		return WicBitmapRenderTarget();

	const D2D1_PIXEL_FORMAT format =
		D2D1::PixelFormat(
			DXGI_FORMAT_B8G8R8A8_UNORM,
			D2D1_ALPHA_MODE_PREMULTIPLIED);
	const D2D1_RENDER_TARGET_PROPERTIES properties =
		D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			format,
			dpi_scale * USER_DEFAULT_SCREEN_DPI,
			dpi_scale * USER_DEFAULT_SCREEN_DPI,
			D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE);
	hr = _factory->CreateWicBitmapRenderTarget(rt.bitmap.Get(),
		properties,
		(ID2D1RenderTarget**)rt.target.GetAddressOf());
	if (!SUCCEEDED(hr))
		return WicBitmapRenderTarget();

	hr = rt.target.As<ID2D1GdiInteropRenderTarget>(&rt.interop_target);
	if (!SUCCEEDED(hr))
		return WicBitmapRenderTarget();

	return rt;
}

ComPtr<IDWriteTextLayout> GraphicDevice::CreateTextLayout(
	const std::wstring& text,
	const std::string& font_family,
	float font_size,
	FontWeight font_weight,
	FontStyle font_style) {

	ComPtr<IDWriteTextLayout> layout;
	ComPtr<IDWriteTextFormat> format;
	HRESULT hr;
	std::wstring utf16_font_family = EncodingManager::UTF8ToWide(font_family);
	DWRITE_FONT_STYLE dwrite_font_style = DWRITE_FONT_STYLE_NORMAL;
	if (font_style == FontStyle::ITALIC)
		dwrite_font_style = DWRITE_FONT_STYLE_ITALIC;
	hr = _dwrite->CreateTextFormat(utf16_font_family.c_str(),
		NULL,
		(DWRITE_FONT_WEIGHT)font_weight.GetRaw(),
		dwrite_font_style,
		DWRITE_FONT_STRETCH_NORMAL,
		font_size,
		DEFAULT_LOCALE,
		format.GetAddressOf());
	hr = _dwrite->CreateTextLayout(text.c_str(), text.length(), format.Get(),
		std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), layout.GetAddressOf());
	return layout;
}

bool GraphicDevice::GetFontMetrics(const std::string& font_family, DWRITE_FONT_METRICS& out_metrics) {
	std::wstring utf16_font_family = EncodingManager::UTF8ToWide(font_family);
	HRESULT hr;
	UINT index;
	BOOL exists = FALSE;
	_font_collection->FindFamilyName(utf16_font_family.c_str(), &index, &exists);
	if (exists) {
		ComPtr<IDWriteFontFamily> family;
		hr = _font_collection->GetFontFamily(index, family.GetAddressOf());
		if (SUCCEEDED(hr)) {
			ComPtr<IDWriteFont> font;
			hr = family->GetFirstMatchingFont(DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				font.GetAddressOf());
			font->GetMetrics(&out_metrics);
			return true;
		}
	}
	return false;
}

void GraphicDevice::LoadBitmapToCache(const std::string& name, absl::Span<uint8_t> res_x1) {
	BitmapItem bitmap;
	bitmap.x1 = LoadBitmapFromResource(res_x1, scene2d::PointF::fromAll(1.0f));
	bitmap.x1_5 = bitmap.x1;
	bitmap.x2 = bitmap.x1;
	_bitmap_cache[name] = bitmap;
}
void GraphicDevice::LoadBitmapToCache(const std::string& name, absl::Span<uint8_t> res_x1,
	absl::Span<uint8_t> res_x1_5, absl::Span<uint8_t> res_x2) {
	BitmapItem bitmap;
	bitmap.x1 = LoadBitmapFromResource(res_x1, scene2d::PointF::fromAll(1.0f));
	bitmap.x1_5 = LoadBitmapFromResource(res_x1_5, scene2d::PointF::fromAll(1.5f));
	bitmap.x2 = LoadBitmapFromResource(res_x2, scene2d::PointF::fromAll(2.0f));
	_bitmap_cache[name] = bitmap;
}
BitmapSubItem GraphicDevice::LoadBitmapFromResource(
	absl::Span<uint8_t> res, const scene2d::PointF& dpi_scale) {
	HRESULT hr;
	ComPtr<IWICStream> stream;
	BitmapSubItem item;
	item.dpi_scale = dpi_scale;
	hr = _wic_factory->CreateStream(stream.GetAddressOf());
	if (FAILED(hr)) {
		return BitmapSubItem();
	}
	hr = stream->InitializeFromMemory((WICInProcPointer)res.data(), res.size());
	if (FAILED(hr)) {
		return BitmapSubItem();
	}
	ComPtr<IWICBitmapDecoder> decoder;
	hr = _wic_factory->CreateDecoderFromStream(stream.Get(), NULL, WICDecodeMetadataCacheOnLoad, decoder.GetAddressOf());
	if (FAILED(hr)) {
		return BitmapSubItem();
	}
	hr = decoder->GetFrame(0, item.frame.GetAddressOf());
	if (FAILED(hr)) {
		return BitmapSubItem();
	}
	hr = _wic_factory->CreateFormatConverter(item.converter.GetAddressOf());
	if (FAILED(hr)) {
		return BitmapSubItem();
	}
	hr = item.converter->Initialize(item.frame.Get(),
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		NULL,
		0.f,
		WICBitmapPaletteTypeMedianCut);
	if (FAILED(hr)) {
		return BitmapSubItem();
	}
	return item;
}
BitmapSubItem GraphicDevice::GetBitmap(const std::string& name, float dpi_scale) const {
	auto it = _bitmap_cache.find(name);
	if (it == _bitmap_cache.end())
		return BitmapSubItem();
	if (dpi_scale >= 1.75f)
		return it->second.x2;
	else if (dpi_scale >= 1.25f)
		return it->second.x1_5;
	else
		return it->second.x1;
}

float GraphicDevice::GetInitialDesktopDpiScale() const {
	FLOAT x, y;
	_factory->GetDesktopDpi(&x, &y);
	(void)y;
	return x / USER_DEFAULT_SCREEN_DPI;
}

} // namespace graphics
} // namespace windows
