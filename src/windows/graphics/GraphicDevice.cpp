// #pragma warning(disable:4996)
#include "GraphicDevice.h"
#include "base/log.h"
#include "windows/EncodingManager.h"
#include "windows/ResourceManager.h"
#include "TextAnalysis.h"
#include "CustomFont.h"
#include "absl/strings/match.h"
#include <numeric>

namespace windows {
namespace graphics {

static const wchar_t* DEFAULT_LOCALE = L"zh-CN";
static const char* DEFAULT_FONT_FAMILY = "Microsoft YaHei";

static GraphicDevice *gdev = nullptr;

GraphicDevice::~GraphicDevice()
{
	_dwrite->UnregisterFontFileLoader(CDwFontFileLoader::getLoader());
}

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
	_dwrite->RegisterFontFileLoader(CDwFontFileLoader::getLoader());

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

std::unique_ptr<TextFlow> GraphicDevice::CreateTextFlow(
	const std::wstring& text,
	float line_height,
	const std::string& font_family,
	float font_size,
	FontWeight font_weight,
	FontStyle font_style)
{
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
	std::unique_ptr<TextFlow> flow = std::make_unique<TextFlow>(_dwrite);
	flow->setTextFormat(format);
	flow->AnalyzeText(text.c_str(), text.length());
	return flow;
}

void GraphicDevice::UpdateTextFlow(TextFlow* text_flow,
	const std::wstring& text,
	float line_height,
	const std::string& font_family,
	float font_size,
	FontWeight font_weight,
	FontStyle font_style)
{
	std::wstring utf16_font_family = EncodingManager::UTF8ToWide(font_family);
	DWRITE_FONT_STYLE dwrite_font_style = DWRITE_FONT_STYLE_NORMAL;
	if (font_style == FontStyle::ITALIC)
		dwrite_font_style = DWRITE_FONT_STYLE_ITALIC;
	ComPtr<IDWriteTextFormat> old_format = text_flow->textFormat();
	WCHAR old_family[256] = {};
	old_format->GetFontFamilyName(old_family, ABSL_ARRAYSIZE(old_family) - 1);
	FLOAT old_font_size = old_format->GetFontSize();
	DWRITE_FONT_WEIGHT old_font_weight = old_format->GetFontWeight();
	DWRITE_FONT_STYLE old_font_style = old_format->GetFontStyle();
	if (utf16_font_family != old_family
		|| font_size != old_font_size
		|| font_weight.GetRaw() != old_font_weight
		|| dwrite_font_style != old_font_style) {
		ComPtr<IDWriteTextFormat> format;
		HRESULT hr;
		hr = _dwrite->CreateTextFormat(utf16_font_family.c_str(),
			NULL,
			(DWRITE_FONT_WEIGHT)font_weight.GetRaw(),
			dwrite_font_style,
			DWRITE_FONT_STRETCH_NORMAL,
			font_size,
			DEFAULT_LOCALE,
			format.GetAddressOf());
		text_flow->setTextFormat(format);
	}
	if (!text_flow)
		return;
	if (text_flow->text() != text) {
		text_flow->AnalyzeText(text.c_str(), text.length());
	}
}

std::string GraphicDevice::GetDefaultFontFamily() const
{
	return DEFAULT_FONT_FAMILY;
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

void GraphicDevice::LoadBitmapToCache(const std::string& name, absl::Span<const uint8_t> res_x1) {
	BitmapItem bitmap;
	bitmap.x1 = LoadBitmapFromResource(res_x1, scene2d::PointF::fromAll(1.0f));
	bitmap.x1_5 = bitmap.x1;
	bitmap.x2 = bitmap.x1;
	_bitmap_cache[name] = bitmap;
}
void GraphicDevice::LoadBitmapToCache(const std::string& name, absl::Span<const uint8_t> res_x1,
	absl::Span<const uint8_t> res_x1_5, absl::Span<const uint8_t> res_x2) {
	BitmapItem bitmap;
	bitmap.x1 = LoadBitmapFromResource(res_x1, scene2d::PointF::fromAll(1.0f));
	bitmap.x1_5 = LoadBitmapFromResource(res_x1_5, scene2d::PointF::fromAll(1.5f));
	bitmap.x2 = LoadBitmapFromResource(res_x2, scene2d::PointF::fromAll(2.0f));
	_bitmap_cache[name] = bitmap;
}
void GraphicDevice::LoadBitmapToCache(const std::string& name, const std::wstring& filename)
{
	BitmapItem bitmap;
	bitmap.x1 = LoadBitmapFromFilename(filename, scene2d::PointF::fromAll(1.0f));
	bitmap.x1_5 = bitmap.x1;
	bitmap.x2 = bitmap.x1;
	_bitmap_cache[name] = bitmap;
}
BitmapSubItem GraphicDevice::LoadBitmapFromResource(absl::Span<const uint8_t> res, const scene2d::PointF& dpi_scale) {
	HRESULT hr;
	ComPtr<IWICStream> stream;

	hr = _wic_factory->CreateStream(stream.GetAddressOf());
	if (FAILED(hr)) {
		return BitmapSubItem();
	}
	hr = stream->InitializeFromMemory((WICInProcPointer)res.data(), res.size());
	if (FAILED(hr)) {
		return BitmapSubItem();
	}
	return LoadBitmapFromStream(stream, dpi_scale);
}
BitmapSubItem GraphicDevice::LoadBitmapFromFilename(const std::wstring& filename, const scene2d::PointF& dpi_scale) {
	HRESULT hr;
	ComPtr<IWICStream> stream;

	hr = _wic_factory->CreateStream(stream.GetAddressOf());
	if (FAILED(hr)) {
		return BitmapSubItem();
	}
	hr = stream->InitializeFromFilename(filename.c_str(), GENERIC_READ);
	if (FAILED(hr)) {
		return BitmapSubItem();
	}
	return LoadBitmapFromStream(stream, dpi_scale);
}
BitmapSubItem GraphicDevice::LoadBitmapFromStream(ComPtr<IWICStream> stream, const scene2d::PointF& dpi_scale) {
	if (!stream)
		return BitmapSubItem();

	HRESULT hr;
	BitmapSubItem item;
	item.dpi_scale = dpi_scale;
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
void GraphicDevice::LoadBitmapToCache(const std::string& name)
{
	if (absl::StartsWith(name, ":")) {
		auto RM = windows::ResourceManager::instance();
		std::string name_res = name.substr(1);
		absl::optional<base::ResourceArchive::ResourceItem> x1, x1_5, x2;
		x1 = RM->loadResource(windows::EncodingManager::UTF8ToWide(name_res).c_str());
		int idx = name_res.rfind('.');
		if (idx != std::string::npos) {
			std::string name_res_x1_5 = name_res.substr(0, idx) + "@1.5x" + name_res.substr(idx);
			x1_5 = RM->loadResource(windows::EncodingManager::UTF8ToWide(name_res_x1_5).c_str());
			std::string name_res_x2 = name_res.substr(0, idx) + "@2x" + name_res.substr(idx);
			x2 = RM->loadResource(windows::EncodingManager::UTF8ToWide(name_res_x2).c_str());
		}
		if (!x1.has_value())
			return;
		if (!x1_5.has_value())
			x1_5 = x1;
		if (!x2.has_value())
			x2 = x1_5;
		LoadBitmapToCache(name, absl::MakeSpan(x1->data, x1->size),
			absl::MakeSpan(x1_5->data, x1_5->size), absl::MakeSpan(x2->data, x2->size));
	} else {
		auto filename_x1 = windows::EncodingManager::UTF8ToWide(name);
		absl::optional<std::wstring> filename_x1_5, filename_x2;
		int idx = name.rfind('.');
		if (idx != std::string::npos) {
			filename_x1_5.emplace(windows::EncodingManager::UTF8ToWide(
				name.substr(0, idx) + "@1.5x" + name.substr(idx)));
			filename_x2.emplace(windows::EncodingManager::UTF8ToWide(
				name.substr(0, idx) + "@2x" + name.substr(idx)));
		}
		BitmapItem bi;
		bi.x1 = LoadBitmapFromFilename(filename_x1, scene2d::PointF::fromAll(1.0f));
		if (filename_x1_5.has_value())
			bi.x1_5 = LoadBitmapFromFilename(filename_x1_5.value(), scene2d::PointF::fromAll(1.5f));
		if (filename_x2.has_value())
			bi.x2 = LoadBitmapFromFilename(filename_x2.value(), scene2d::PointF::fromAll(2.0f));
		if (!bi.x1_5)
			bi.x1_5 = bi.x1;
		if (!bi.x2)
			bi.x2 = bi.x1_5;
		_bitmap_cache[name] = bi;
	}
}
BitmapSubItem GraphicDevice::GetBitmap(const std::string& name, float dpi_scale) {
	auto it = _bitmap_cache.find(name);
	if (it == _bitmap_cache.end()) {
		LoadBitmapToCache(name);
		it = _bitmap_cache.find(name);
	}
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

void GraphicDevice::addFont(const char* family_name, const uint8_t* data, size_t size)
{
	std::wstring u16_family_name = EncodingManager::UTF8ToWide(family_name);
	_font_cache[u16_family_name] = CDWriteExt::DwCreateFontFaceFromStream(_dwrite.Get(), data, size, DWRITE_FONT_SIMULATIONS_NONE);
}

ComPtr<IDWriteFontFace> GraphicDevice::getFirstMatchingFontFace(const wchar_t* family_name, DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STRETCH stretch, DWRITE_FONT_STYLE style)
{
	auto it = _font_cache.find(family_name);
	if (it != _font_cache.end()) {
		return it->second;
	}

	UINT32 font_index = 0;
	HRESULT hr = S_OK;
	BOOL font_exists = false;
	hr = _font_collection->FindFamilyName(family_name, &font_index, &font_exists);
	if (!font_exists)
	{
		// If the given font does not exist, take what we can get
		// (displaying something instead nothing), choosing the foremost
		// font in the collection.
		font_index = 0;
	}

	ComPtr<IDWriteFontFamily> font_family;
	if (SUCCEEDED(hr))
	{
		hr = _font_collection->GetFontFamily(font_index, font_family.GetAddressOf());
	}

	ComPtr<IDWriteFont> font;
	if (SUCCEEDED(hr))
	{
		hr = font_family->GetFirstMatchingFont(
			weight,
			stretch,
			style,
			font.GetAddressOf()
		);
	}

	ComPtr<IDWriteFontFace> font_face;
	if (SUCCEEDED(hr))
	{
		hr = font->CreateFontFace(font_face.GetAddressOf());
	}

	return font_face;
}

} // namespace graphics
} // namespace windows
