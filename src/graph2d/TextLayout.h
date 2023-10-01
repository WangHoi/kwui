#pragma once
#include "scene2d/geom_types.h"

namespace graph2d {

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
