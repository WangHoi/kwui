#include "scene2d/TextLayout.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"

namespace xskia {

class GlyphRunX;
class TextLayoutX : public scene2d::TextLayoutInterface {
public:
	TextLayoutX(const std::wstring& text,
		const std::string& font_family,
		float font_size);
	~TextLayoutX();

	// 通过 TextLayoutInterface 继承
	float lineHeight() const override;
	float baseline() const override;
	scene2d::RectF rect() const override;
	scene2d::RectF caretRect(int idx) const override;
	scene2d::RectF rangeRect(int start_idx, int end_idx) const override;
	int hitTest(const scene2d::PointF& pos, scene2d::RectF* out_caret_rect) const override;

	const GlyphRunX* glyphRun() const;

private:
	void computeTextLayout(const std::wstring& u16_text);

	SkFont font_;
	SkFontMetrics font_metrics_;
	std::string text_;
	SkRect rect_;
	std::vector<SkGlyphID> glyphs_;
	std::vector<SkScalar> advances_;
	std::unique_ptr<GlyphRunX> run_;
};

}
