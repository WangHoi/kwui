#include "TextFlowX.h"
#include "base/EncodingManager.h"

namespace xskia {

TextFlowX::TextFlowX(const std::string& text,
	const char* font_family,
	style::FontStyle font_style,
	style::FontWeight font_weight,
	float font_size)
	: font_(SkTypeface::MakeFromName(font_family, SkFontStyle()), font_size)
{
	font_.getMetrics(&font_metrics_);
	text_ = base::EncodingManager::UTF8ToWide(text);
}

TextFlowX::~TextFlowX() {}

style::FontMetrics TextFlowX::fontMetrics()
{
	style::FontMetrics fm;
	fm.ascent = -font_metrics_.fAscent;
	fm.descent = font_metrics_.fDescent;
	fm.line_gap = font_metrics_.fLeading;
	fm.cap_height = font_metrics_.fCapHeight;
	fm.x_height = font_metrics_.fXHeight;
	fm.underline_offset = font_metrics_.fUnderlinePosition;
	fm.underline_thickness = font_metrics_.fUnderlineThickness;
	fm.line_through_offset = font_metrics_.fStrikeoutPosition;
	fm.line_through_thickness = font_metrics_.fStrikeoutThickness;
	return fm;
}
std::tuple<float, float> TextFlowX::measureWidth()
{
	return std::make_tuple(0.0f, 1000.0f);
}
void TextFlowX::flowText(graph2d::TextFlowSourceInterface* source, graph2d::TextFlowSinkInterface* sink)
{
	bool first = true;
	float line_height = -font_metrics_.fAscent + font_metrics_.fDescent + font_metrics_.fLeading;
	source->getCurrentLine(line_height, left, width, empty);
}

}