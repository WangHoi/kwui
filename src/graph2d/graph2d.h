#pragma once

#include "TextLayout.h"
#include "Bitmap.h"
#include "Painter.h"
#include "PaintSurface.h"
#include "style/StyleFont.h"
#include <memory>
#include <string>

namespace graph2d {

std::unique_ptr<TextFlowInterface> createTextFlow(
	const std::string& text,
	float line_height,
	const char* font_family,
	style::FontStyle font_style,
	style::FontWeight font_weight,
	float font_size);
void updateTextFlow(
	std::unique_ptr<TextFlowInterface>& text_flow,
	const std::string& text,
	float line_height,
	const char* font_family,
	style::FontStyle font_style,
	style::FontWeight font_weight,
	float font_size);
style::FontMetrics getFontMetrics(const char* font_family, float font_size);

std::shared_ptr<BitmapInterface> createBitmap(const std::string& url);

}