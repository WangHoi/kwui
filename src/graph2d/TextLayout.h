#pragma once
#include "scene2d/geom_types.h"
#include "absl/types/optional.h"

namespace graph2d {

class GlyphRunInterface {
public:
    virtual scene2d::RectF boundingRect() = 0;
};

class TextFlowSourceInterface {
public:
    virtual absl::optional<scene2d::RectF> getNextRect(float fontHeight) = 0;
};

class TextFlowSinkInterface {
public:
    virtual void prepare(size_t glyph_count) = 0;
    virtual void setGlyphRun(
        const scene2d::PointF& pos,
        std::unique_ptr<GlyphRunInterface> glyph_run) = 0;
};

struct FontMetrics {
    float ascent;
    float descent;
    float line_gap;
};

class TextFlowInterface {
public:
    virtual FontMetrics fontMetrics() = 0;
    virtual std::tuple<float, float> measureWidth() = 0;
    virtual void flowText(TextFlowSourceInterface *source, TextFlowSinkInterface *sink) = 0;
};

class TextLayoutInterface {
public:
    virtual float lineHeight() const = 0;
    virtual float baseline() const = 0;
    virtual scene2d::RectF rect() const = 0;
    virtual scene2d::RectF caretRect(int idx) const = 0;
    virtual scene2d::RectF rangeRect(int start_idx, int end_idx) const = 0;
    virtual int hitTest(const scene2d::PointF& pos, scene2d::RectF* out_caret_rect = nullptr) const = 0;
};

}
