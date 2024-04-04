#pragma once
#include "graph2d/TextLayout.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"

namespace xskia {

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
    std::wstring text_;
};

}
