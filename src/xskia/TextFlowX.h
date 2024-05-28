#pragma once
#include "style/TextFlow.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include <vector>

namespace xskia {

class PainterX;
class GlyphRunX : public style::GlyphRunInterface {
public:
    GlyphRunX(SkFont font, const SkFontMetrics& fm, absl::Span<SkGlyphID> glyphs, absl::Span<SkScalar> advances);
    ~GlyphRunX() override {}
    scene2d::RectF boundingRect() override;

private:
    SkFont font_;
    scene2d::RectF bounds_;
    std::vector<SkGlyphID> glyphs_;
    std::vector<SkPoint> positions_;

    friend class PainterX;
};

class TextFlowX : public style::TextFlowInterface {
public:
    TextFlowX(const std::string& text,
        const char* font_family,
        style::FontStyle font_style,
        style::FontWeight font_weight,
        float font_size);
    ~TextFlowX() override;
    style::FontMetrics fontMetrics() override;
    std::tuple<float, float> measureWidth() override;
    void flowText(style::TextFlowSourceInterface* source, style::TextFlowSinkInterface* sink) override;

private:
    SkFont font_;
    SkFontMetrics font_metrics_;
    std::string text_;
};

}
