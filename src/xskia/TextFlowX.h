#pragma once
#include "graph2d/TextLayout.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include <vector>

namespace xskia {

class PainterX;
class GlyphRunX : public graph2d::GlyphRunInterface {
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

class TextFlowX : public graph2d::TextFlowInterface {
public:
    TextFlowX(const std::string& text,
        const char* font_family,
        style::FontStyle font_style,
        style::FontWeight font_weight,
        float font_size);
    ~TextFlowX() override;
    style::FontMetrics fontMetrics() override;
    std::tuple<float, float> measureWidth() override;
    void flowText(graph2d::TextFlowSourceInterface* source, graph2d::TextFlowSinkInterface* sink) override;

private:
    SkFont font_;
    SkFontMetrics font_metrics_;
    std::string text_;
};

}
