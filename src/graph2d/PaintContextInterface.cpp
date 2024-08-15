#include "PaintContextInterface.h"
#include "style/Layout.h"

namespace graph2d
{
void PaintContextInterface::drawBox(const scene2d::RectF& padding_rect, const style::EdgeOffsetF& border_width,
                                    const scene2d::CornerRadiusF& border_radius, const style::Color& background_color,
                                    const style::Color& border_color,
                                    const graph2d::BitmapInterface* background_image)
{
    // Expand padding_rect to get the border-box
    auto outer = scene2d::RRectF::fromRectRadius(padding_rect, border_radius);
    outer.rect.adjust(-border_width.left, -border_width.top, border_width.right, border_width.bottom);

    // Fill
    if (background_color.getAlpha() > 0 || background_image) {
        graph2d::PaintBrush brush;
        brush.setColor(background_color);
        brush.setShader(background_image);
        drawRRect(outer, brush);
    }

    // Check stroke width
    float max_border_width = std::max({border_width.left, border_width.top, border_width.right, border_width.bottom});
    if (max_border_width > 0) {
        bool equal_border_width = (border_width.left == border_width.top
            && border_width.left == border_width.right
            && border_width.left == border_width.bottom);
        if (equal_border_width) {
            scene2d::RRectF border_rrect = outer;
            border_rrect.rect.adjust(0.5f * max_border_width, 0.5f * max_border_width,
                                     -0.5f * max_border_width, -0.5f * max_border_width);
            auto& r = border_rrect.corner_radius;
            r.top_left.width = std::max(0.0f, r.top_left.width - 0.5f * max_border_width);
            r.top_left.height = std::max(0.0f, r.top_left.height - 0.5f * max_border_width);
            r.top_right.width = std::max(0.0f, r.top_right.width - 0.5f * max_border_width);
            r.top_right.height = std::max(0.0f, r.top_right.height - 0.5f * max_border_width);
            r.bottom_right.width = std::max(0.0f, r.bottom_right.width - 0.5f * max_border_width);
            r.bottom_right.height = std::max(0.0f, r.bottom_right.height - 0.5f * max_border_width);
            r.bottom_left.width = std::max(0.0f, r.bottom_left.width - 0.5f * max_border_width);
            r.bottom_left.height = std::max(0.0f, r.bottom_left.height - 0.5f * max_border_width);
            graph2d::PaintBrush brush;
            brush.setStyle(graph2d::PAINT_STYLE_STROKE);
            brush.setStrokeWidth(max_border_width);
            brush.setColor(border_color);
            drawRRect(border_rrect, brush);
        } else {
            auto inner = scene2d::RRectF::fromRectRadius(padding_rect, border_radius);
            auto& r = inner.corner_radius;
            r.top_left.width = std::max(0.0f, r.top_left.width - 0.5f * border_width.left);
            r.top_left.height = std::max(0.0f, r.top_left.height - 0.5f * border_width.top);
            r.top_right.width = std::max(0.0f, r.top_right.width - 0.5f * border_width.right);
            r.top_right.height = std::max(0.0f, r.top_right.height - 0.5f * border_width.top);
            r.bottom_right.width = std::max(0.0f, r.bottom_right.width - 0.5f * border_width.right);
            r.bottom_right.height = std::max(0.0f, r.bottom_right.height - 0.5f * border_width.bottom);
            r.bottom_left.width = std::max(0.0f, r.bottom_left.width - 0.5f * border_width.left);
            r.bottom_left.height = std::max(0.0f, r.bottom_left.height - 0.5f * border_width.bottom);
            graph2d::PaintBrush brush;
            brush.setColor(border_color);
            drawDRRect(outer, inner, brush);
        }
    }
}
}
