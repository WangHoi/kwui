#pragma once
#include "scene2d/geom_types.h"
#include "style/StyleValue.h"
#include "style/StyleColor.h"

namespace graph2d
{

class BitmapInterface;

enum PaintStyle
{
    PAINT_STYLE_FILL,
    PAINT_STYLE_STROKE,
};
enum StrokeCap
{
    /// Begin and end contours with a flat edge and no extension.
    STROKE_CAP_BUTT,
    /// Begin and end contours with a semi-circle extension.
    STROKE_CAP_ROUND,
    /// Begin and end contours with a half square extension.
    /// This is similar to extending each contour by half the stroke width.
    STROKE_CAP_SQUARE,
};
enum StrokeJoin
{
    /// Joins between line segments form sharp corners.
    STROKE_JOIN_MITER,
    /// Joins between line segments are semi-circular.
    STROKE_JOIN_ROUND,
    /// Joins between line segments connect the corners of the butt ends of the line segments to give a beveled appearance.
    STROKE_JOIN_BEVEL,
};
class PaintBrush
{
public:
    PaintStyle style() const { return style_; }
    void setStyle(PaintStyle style) { style_ = style; }
    const style::Color& color() const { return color_; }
    void setColor(const style::Color& c) { color_ = c; }
    float strokeWidth() const { return stroke_width_; }
    void setStrokeWidth(float width) { stroke_width_ = width; }
    StrokeCap strokeCap() const { return stroke_cap_; }
    void setStrokeCap(StrokeCap cap) { stroke_cap_ = cap; }
    StrokeJoin strokeJoin() const { return stroke_join_; }
    void setStrokeJoin(StrokeJoin join) { stroke_join_ = join; }
    float strokeMiterLimit() const { return stroke_miter_limit_; }
    void setStrokeMiterLimit(float limit) { stroke_miter_limit_ = limit; }
    const BitmapInterface* shader() const { return shader_; }
    void setShader(const BitmapInterface* shader) { shader_ = shader; }

private:
    PaintStyle style_ = PAINT_STYLE_FILL;
    style::Color color_;
    float stroke_width_ = 0.0f;
    StrokeCap stroke_cap_ = STROKE_CAP_BUTT;
    StrokeJoin stroke_join_ = STROKE_JOIN_MITER;
    float stroke_miter_limit_ = 4.0f;
    const BitmapInterface* shader_ = nullptr;
};
}
