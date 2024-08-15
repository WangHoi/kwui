#pragma once
#include "scene2d/geom_types.h"
#include "style/StyleValue.h"
#include "style/StyleColor.h"
#include "PaintBrush.h"

namespace scene2d
{
class Control;
class TextLayoutInterface;
struct CornerRadiusF;
}

namespace style
{
struct EdgeOffsetF;
class GlyphRunInterface;
}

namespace graph2d
{
class PaintPathInterface;

class GlyphRunInterface;
class BitmapInterface;

struct BoxShadow
{
    bool inset = false;
    style::Color color;
    float offset_x = 0, offset_y = 0;
    float blur_radius = 0;
    float spread_x = 0, spread_y = 0;
};

class PaintContextInterface
{
public:
    virtual ~PaintContextInterface() = default;
    virtual void clear(const style::Color& c) = 0;
    virtual void save() = 0;
    virtual void restore() = 0;
    virtual float getDpiScale() = 0;
    virtual void translate(const scene2d::PointF& offset) = 0;
    void translate(float x, float y) { translate({x, y}); }
    // virtual void rotate(float radians, const scene2d::PointF& center) = 0;
    // void rotate(float radians) { rotate(radians, scene2d::PointF()); }
    // virtual void scale(const scene2d::PointF& s) = 0;
    // void scale(float sx, float sy) { scale({sx, sy}); }
    virtual void clipRect(const scene2d::RectF& rect) = 0;

    void clipRect(const scene2d::PointF& origin, const scene2d::DimensionF& size)
    {
        clipRect(scene2d::RectF::fromOriginSize(origin, size));
    }

    virtual void drawRect(const scene2d::RectF& rect, const PaintBrush& brush) = 0;
    virtual void drawRRect(const scene2d::RRectF& rrect, const PaintBrush& brush) = 0;
    virtual void drawDRRect(const scene2d::RRectF& outer, const scene2d::RRectF& inner, const PaintBrush& brush) = 0;
    virtual void drawPath(const PaintPathInterface* path, const PaintBrush& brush) = 0;
    void drawBox(const scene2d::RectF& padding_rect,
                 const style::EdgeOffsetF& inset_border_width,
                 const scene2d::CornerRadiusF& border_radius,
                 const style::Color& background_color,
                 const style::Color& border_color,
                 const BitmapInterface* background_image = nullptr);
    virtual void drawBoxShadow(const scene2d::RectF& padding_rect,
                               const style::EdgeOffsetF& inset_border_width,
                               const scene2d::CornerRadiusF& border_radius,
                               const BoxShadow& box_shadow) = 0;
    virtual void drawGlyphRun(const scene2d::PointF& pos,
                              const style::GlyphRunInterface* text_flow,
                              const style::Color& color) = 0;
    virtual void drawControl(const scene2d::RectF& rect,
                             scene2d::Control* control) = 0;
    virtual void drawBitmap(const BitmapInterface* image,
                            const scene2d::PointF& origin,
                            const scene2d::DimensionF& size) = 0;
    // TODO: drawRoundedRect(): support stroke
    virtual void drawRoundedRect(const scene2d::RectF& rect,
                                 const scene2d::CornerRadiusF& border_radius,
                                 const style::Color& background_color) = 0;
    // TODO: drawRect(): support stroke
    virtual void drawRect(const scene2d::RectF& rect,
                          const style::Color& background_color) = 0;
    virtual void drawTextLayout(const scene2d::PointF& origin,
                                const scene2d::TextLayoutInterface& layout,
                                const style::Color& color) = 0;
    virtual void drawCircle(const scene2d::PointF& center,
                            float radius,
                            const style::Color& background_color,
                            float border_width,
                            const style::Color& border_color) = 0;
    virtual void drawArc(const scene2d::PointF& center,
                         float radius,
                         float start_angle,
                         float span_angle,
                         const style::Color& background_color,
                         float border_width,
                         const style::Color& border_color) = 0;
};
}
