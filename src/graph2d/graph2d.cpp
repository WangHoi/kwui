#include "graph2d.h"
#ifdef _WIN32
#include "windows/graphics/GraphicDeviceD2D.h"
#include "windows/graphics/TextLayoutD2D.h"
#include "windows/graphics/TextFlowD2D.h"
#include "windows/graphics/PainterD2D.h"
#include "windows/windows_header.h"
#endif
#if WITH_SKIA
#include "xskia/TextFlowX.h"
#include "xskia/GraphicDeviceX.h"
#include "xskia/BitmapX.h"
#include "xskia/PaintPathX.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkStream.h"
#endif
#include "base/EncodingManager.h"

namespace graph2d {

std::unique_ptr<style::TextFlowInterface> createTextFlow(
	const std::string& text,
	float line_height,
	const char* font_family,
	style::FontStyle font_style,
	style::FontWeight font_weight,
	float font_size)
{
#if WITH_SKIA
	return std::make_unique<xskia::TextFlowX>(text, font_family, font_style, font_weight, font_size);
#else
#ifdef _WIN32
	std::wstring u16_text = base::EncodingManager::UTF8ToWide(text);
	return windows::graphics::GraphicDeviceD2D::instance()
		->createTextFlow(u16_text, line_height, font_family, font_size, font_weight, font_style);
#else
#pragma message("TODO: implement graph2d::createTextFlow().")
	return nullptr;
#endif
#endif
}

void updateTextFlow(
	std::unique_ptr<style::TextFlowInterface>& text_flow,
	const std::string& text,
	float line_height,
	const char* font_family,
	style::FontStyle font_style,
	style::FontWeight font_weight,
	float font_size)
{
#if WITH_SKIA
	text_flow = createTextFlow(text, line_height, font_family, font_style, font_weight, font_size);
#else
#ifdef _WIN32
	std::wstring u16_text = base::EncodingManager::UTF8ToWide(text);
	windows::graphics::TextFlowD2D* win_tf = static_cast<windows::graphics::TextFlowD2D*>(text_flow.get());
	return windows::graphics::GraphicDeviceD2D::instance()
		->updateTextFlow(win_tf, u16_text, line_height, font_family, font_size, font_weight, font_style);
#else
	text_flow = createTextFlow(text, line_height, font_family, font_style, font_weight, font_size);
#endif
#endif
}

style::FontMetrics getFontMetrics(const char* font_family, float font_size)
{
#if WITH_SKIA
	style::FontMetrics fm;
	auto font_face = xskia::GraphicDeviceX::instance()
		->getFirstMatchingFontFace(font_family, SkFontStyle());
	SkFont fnt(font_face, font_size);
	SkFontMetrics sfm;
	fnt.getMetrics(&sfm);
	fm.ascent = -sfm.fAscent;
	fm.descent = sfm.fDescent;
	fm.line_gap = sfm.fLeading;
	fm.cap_height = sfm.fCapHeight;
	fm.x_height = sfm.fXHeight;
	fm.underline_offset = -sfm.fUnderlinePosition;
	fm.underline_thickness = sfm.fUnderlineThickness;
	fm.line_through_offset = -sfm.fStrikeoutPosition;
	fm.line_through_thickness = sfm.fStrikeoutThickness;
	return fm;
#else
#ifdef _WIN32
	style::FontMetrics fm;
	DWRITE_FONT_METRICS dwrite_fm;
	auto GD = windows::graphics::GraphicDeviceD2D::instance();
	bool ok = GD->getFontMetrics(font_family, dwrite_fm);
	if (!ok) {
		std::string default_font = GD->getDefaultFontFamily();
		GD->getFontMetrics(default_font, dwrite_fm);
	}
	const float factor = font_size / dwrite_fm.designUnitsPerEm;
	fm.ascent = dwrite_fm.ascent * factor;
	fm.descent = dwrite_fm.descent * factor;
	fm.line_gap = dwrite_fm.lineGap * factor;
	fm.cap_height = dwrite_fm.capHeight * factor;
	fm.x_height = dwrite_fm.xHeight * factor;
	fm.underline_offset = dwrite_fm.underlinePosition * factor;
	fm.underline_thickness = dwrite_fm.underlineThickness * factor;
	fm.line_through_offset = dwrite_fm.strikethroughPosition * factor;
	fm.line_through_thickness = dwrite_fm.strikethroughThickness * factor;
	return fm;
#else
#pragma message("TODO: implement graph2d::createTextFlow().")
	return style::FontMetrics();
#endif
#endif
}
void addFont(const char* family_name, const void* data, size_t size)
{
#if WITH_SKIA
	xskia::GraphicDeviceX::instance()
		->addFont(family_name, data, size);
#else
#ifdef _WIN32
	windows::graphics::GraphicDeviceD2D::instance()
		->addFont(family_name, (const uint8_t*)data, size);
#else
#pragma message("TODO: implement graph2d::addFont().")
#endif
#endif
}
float getInitialDesktopDpiScale()
{
#ifdef _WIN32
	HDC screen = GetDC(NULL);
	float hPixelsPerInch = GetDeviceCaps(screen, LOGPIXELSX);
	ReleaseDC(NULL, screen);
	return hPixelsPerInch / 96.0f;
#else
#pragma message("TODO: graph2d::getInitialDesktopDpiScale().")
	return 1.0f;
#endif
}
std::shared_ptr<BitmapInterface> createBitmapFromUrl(const std::string& url)
{
#if WITH_SKIA
	return std::shared_ptr<BitmapInterface>(new xskia::BitmapFromUrlX(url));
#else
#ifdef _WIN32
	return std::shared_ptr<BitmapInterface>(new windows::graphics::BitmapFromUrlD2D(url));
#else
#pragma message("TODO: implement graph2d::createBitmapFromUrl().")
	return nullptr;
#endif
#endif
}

std::shared_ptr<BitmapInterface> createBitmap(const void* pixels, size_t pixel_width, size_t pixel_height,
											  size_t src_stride, kwui::ColorType color_type, float dpi_scale)
{
#if WITH_SKIA
	SkColorType format;
	SkAlphaType alpha;
	switch (color_type) {
	case kwui::COLOR_TYPE_ALPHA8:
		format = SkColorType::kAlpha_8_SkColorType;
		alpha = SkAlphaType::kUnknown_SkAlphaType;
		break;
	case kwui::COLOR_TYPE_BGRA8888:
		format = SkColorType::kBGRA_8888_SkColorType;
		alpha = SkAlphaType::kPremul_SkAlphaType;
		break;
	case kwui::COLOR_TYPE_RGBA8888:
		format = SkColorType::kRGBA_8888_SkColorType;
		alpha = SkAlphaType::kPremul_SkAlphaType;
		break;
	default:
		return nullptr;
	}

	SkImageInfo info = SkImageInfo::Make(pixel_width, pixel_height, format, alpha);
	auto data = SkData::MakeWithCopy(pixels, src_stride * pixel_height);
	auto img = SkImage::MakeRasterData(info, data, src_stride);
	return std::make_shared<xskia::BitmapX>(img, dpi_scale);
#else
#ifdef _WIN32
	DXGI_FORMAT format;
	D2D1_ALPHA_MODE alpha;
	switch (color_type) {
	case kwui::COLOR_TYPE_ALPHA8:
		format = DXGI_FORMAT_A8_UNORM;
		alpha = D2D1_ALPHA_MODE_IGNORE;
		break;
	case kwui::COLOR_TYPE_BGRA8888:
		format = DXGI_FORMAT_B8G8R8A8_UNORM;
		alpha = D2D1_ALPHA_MODE_PREMULTIPLIED;
		break;
	case kwui::COLOR_TYPE_RGBA8888:
		format = DXGI_FORMAT_R8G8B8A8_UNORM;
		alpha = D2D1_ALPHA_MODE_PREMULTIPLIED;
		break;
	default:
		return nullptr;
	}
	return std::shared_ptr<BitmapInterface>(new windows::graphics::BitmapD2D(
		pixels, pixel_width, pixel_height, src_stride, dpi_scale, format, alpha));
#else
#pragma message("TODO: implement graph2d::createBitmap().")
	return nullptr;
#endif
#endif
}


std::unique_ptr<PaintPathInterface> createPath()
{
#if WITH_SKIA
	return std::unique_ptr<xskia::PaintPathX>(new xskia::PaintPathX);
#else
#ifdef _WIN32
	return std::unique_ptr<PaintPathInterface>(new windows::graphics::PaintPathD2D());
#else
#pragma message("TODO: implement graph2d::createPath().")
	return nullptr;
#endif
#endif
}
}
