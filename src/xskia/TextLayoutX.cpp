#include "TextLayoutX.h"
#include "TextFlowX.h"
#include "GraphicDeviceX.h"
#include "base/EncodingManager.h"

namespace xskia {
TextLayoutX::TextLayoutX(const std::wstring& text,
	const std::string& font_family,
	float font_size)
{
	// TODO: custom font style
	auto font_face = GraphicDeviceX::instance()
		->getFirstMatchingFontFace(font_family.c_str(), SkFontStyle());
	font_ = SkFont(font_face, font_size);
	font_.getMetrics(&font_metrics_);

	computeTextLayout(text);
}
float TextLayoutX::lineHeight() const
{
	return -font_metrics_.fAscent + font_metrics_.fDescent + font_metrics_.fLeading;
}
float TextLayoutX::baseline() const
{
	return -font_metrics_.fAscent + 0.5f * font_metrics_.fLeading;
}
scene2d::RectF TextLayoutX::rect() const
{
	return scene2d::RectF::fromLTRB(rect_.fLeft, rect_.fTop, rect_.fRight, rect_.fBottom);
}
scene2d::RectF TextLayoutX::caretRect(int idx) const
{
	float x = 0.0f;
	idx = std::clamp(idx, 0, (int)advances_.size());
	for (int i = 0; i < idx; ++i) {
		x += advances_[i];
	}
	return scene2d::RectF::fromXYWH(x, 0, 1, lineHeight());
}
scene2d::RectF TextLayoutX::rangeRect(int start_idx, int end_idx) const
{
	if (start_idx > end_idx)
		std::swap(start_idx, end_idx);
	
	float x1 = 0.0f;
	start_idx = std::clamp(start_idx, 0, (int)advances_.size());
	end_idx = std::clamp(end_idx, 0, (int)advances_.size());
	for (int i = 0; i < start_idx; ++i) {
		x1 += advances_[i];
	}
	float x2 = x1;
	for (int i = start_idx; i < end_idx; ++i) {
		x2 += advances_[i];
	}
	return scene2d::RectF::fromXYWH(x1, 0, std::max(1.0f, x2 - x1), lineHeight());
}
int TextLayoutX::hitTest(const scene2d::PointF& pos, scene2d::RectF* out_caret_rect) const
{
	if (advances_.empty())
		return 0;
	float x = pos.x;
	for (int i = 0; i < (int)advances_.size(); ++i) {
		if (x <= 0.5f * advances_[i])
			return i;
		x -= advances_[i];
	}
	return advances_.size();
}
const GlyphRunX* TextLayoutX::glyphRun() const
{
	return run_.get();
}
void TextLayoutX::computeTextLayout(const std::wstring& u16_text)
{
	text_ = base::EncodingManager::WideToUTF8(u16_text);
	size_t glyph_count = font_.countText(text_.c_str(), text_.length(), SkTextEncoding::kUTF8);
	glyphs_.resize(glyph_count);
	advances_.resize(glyph_count);
	font_.textToGlyphs(text_.c_str(), text_.length(), SkTextEncoding::kUTF8, glyphs_.data(), glyph_count);
	font_.getWidthsBounds(glyphs_.data(), glyph_count, advances_.data(), nullptr, nullptr);
	font_.measureText(text_.c_str(), text_.length(), SkTextEncoding::kUTF8, &rect_);
	run_ = std::make_unique<GlyphRunX>(
		font_,
		font_metrics_,
		absl::Span<SkGlyphID>(glyphs_),
		absl::Span<SkScalar>(advances_));
}
}
