#pragma once
#include "scene2d/geom_types.h"
#include "absl/types/optional.h"

namespace style {
struct LineBox;
}

namespace graph2d {

class GlyphRunInterface {
public:
    virtual scene2d::RectF boundingRect() = 0;
};

class TextFlowSourceInterface {
public:
    virtual style::LineBox* getCurrentLine(float fontHeight, float& left, float& width, bool& empty) = 0;
    virtual style::LineBox* getNextLine(float fontHeight, float& left, float& width) = 0;
};

class TextFlowSinkInterface {
public:
    virtual void prepare(size_t glyph_count) = 0;
    virtual void addGlyphRun(
        style::LineBox* line,
        const scene2d::PointF& pos,
        std::unique_ptr<GlyphRunInterface> glyph_run) = 0;
};

struct FlowMetrics {
    float line_height;
    float baseline;
};

class TextFlowInterface {
public:
    virtual FlowMetrics flowMetrics() = 0;
    virtual std::tuple<float, float> measureWidth() = 0;
    virtual void flowText(TextFlowSourceInterface *source, TextFlowSinkInterface *sink) = 0;
};

}
