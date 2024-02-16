#pragma once
#include "scene2d/geom_types.h"
#include "absl/types/optional.h"
#include "style/StyleFont.h"
#include "style/StyleValue.h"

namespace style {
struct LineBox;
}

namespace graph2d {

class GlyphRunInterface {
public:
    virtual ~GlyphRunInterface() = default;
    virtual scene2d::RectF boundingRect() = 0;
};

class TextFlowSourceInterface {
public:
    virtual ~TextFlowSourceInterface() = default;
    virtual style::LineBox* getCurrentLine(float fontHeight, float& left, float& width, bool& empty) = 0;
    virtual style::LineBox* getNextLine(float fontHeight, float& left, float& width) = 0;
};

class TextFlowSinkInterface {
public:
    virtual ~TextFlowSinkInterface() = default;
    virtual void prepare(size_t glyph_count) = 0;
    virtual void addGlyphRun(
        style::LineBox* line,
        const scene2d::PointF& pos,
        std::unique_ptr<GlyphRunInterface> glyph_run) = 0;
};

class TextFlowInterface {
public:
    virtual ~TextFlowInterface() = default;
    virtual style::FontMetrics fontMetrics() = 0;
    virtual std::tuple<float, float> measureWidth() = 0;
    virtual void flowText(TextFlowSourceInterface *source, TextFlowSinkInterface *sink) = 0;
};

}
