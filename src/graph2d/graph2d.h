#pragma once

#include "Bitmap.h"
#include "PaintContextInterface.h"
#include "PaintSurface.h"
#include "style/StyleFont.h"
#include "style/TextFlow.h"
#include <memory>
#include <string>

namespace graph2d {

std::unique_ptr<style::TextFlowInterface> createTextFlow(
	const std::string& text,
	float line_height,
	const char* font_family,
	style::FontStyle font_style,
	style::FontWeight font_weight,
	float font_size);
void updateTextFlow(
	std::unique_ptr<style::TextFlowInterface>& text_flow,
	const std::string& text,
	float line_height,
	const char* font_family,
	style::FontStyle font_style,
	style::FontWeight font_weight,
	float font_size);
style::FontMetrics getFontMetrics(const char* font_family, float font_size);
void addFont(const char* family_name, const void* data, size_t size);
float getInitialDesktopDpiScale();
std::shared_ptr<BitmapInterface> createBitmap(const std::string& url);

}