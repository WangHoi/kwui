#pragma once

#include "TextLayout.h"
#include "Painter.h"
#include "style/StyleValue.h"
#include <memory>
#include <string>

namespace graph2d {

std::unique_ptr<TextFlowInterface> createTextFlow(
	const std::string& text,
	const char* font_family,
	style::FontStyle font_style,
	style::FontWeight font_weight,
	float font_size);

}