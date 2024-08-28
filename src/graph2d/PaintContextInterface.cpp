#include "PaintContextInterface.h"
#include "Bitmap.h"
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

void PaintContextInterface::drawBitmapNine(const BitmapInterface* image, const style::EdgeOffsetF& src_center,
                                           const scene2d::RectF& dst_rect)
{
    auto src_dpi_scale = image->dpiScale(getDpiScale());
    auto src_resolution = image->pixelSize() /  image->dpiScale(getDpiScale());
    // Top-Left
    drawBitmapRect(image, scene2d::RectF::fromLTRB(0, 0,
                                                   src_center.left, src_center.top),
                   scene2d::RectF::fromLTRB(dst_rect.left, dst_rect.top,
                                            dst_rect.left + src_center.left, dst_rect.top + src_center.top));
    // Top
    drawBitmapRect(image, scene2d::RectF::fromLTRB(src_center.left, 0,
                                                   src_resolution.width - src_center.right, src_center.top),
                   scene2d::RectF::fromLTRB(dst_rect.left + src_center.left, dst_rect.top,
                                            dst_rect.right - src_center.right, dst_rect.top + src_center.top));
    // Top-Right
    drawBitmapRect(image, scene2d::RectF::fromLTRB(src_resolution.width - src_center.right, 0,
                                                   src_resolution.width, src_center.top),
                   scene2d::RectF::fromLTRB(dst_rect.right - src_center.right, dst_rect.top,
                                            dst_rect.right, dst_rect.top + src_center.top));
    // Left
    drawBitmapRect(image, scene2d::RectF::fromLTRB(0, src_center.top,
                                                   src_center.left, src_resolution.height - src_center.bottom),
                   scene2d::RectF::fromLTRB(dst_rect.left, dst_rect.top + src_center.top,
                                            dst_rect.left + src_center.left, dst_rect.bottom - src_center.bottom));
    // Center
    drawBitmapRect(image, scene2d::RectF::fromLTRB(src_center.left, src_center.top,
                                                   src_resolution.width - src_center.right,
                                                   src_resolution.height - src_center.bottom),
                   scene2d::RectF::fromLTRB(dst_rect.left + src_center.left, dst_rect.top + src_center.top,
                                            dst_rect.right - src_center.right, dst_rect.bottom - src_center.bottom));
    // Right
    drawBitmapRect(
        image, scene2d::RectF::fromLTRB(src_resolution.width - src_center.right, src_center.top,
                                        src_resolution.width, src_resolution.height - src_center.bottom),
        scene2d::RectF::fromLTRB(dst_rect.right - src_center.right, dst_rect.top + src_center.top,
                                 dst_rect.right, dst_rect.bottom - src_center.bottom));
    // Bottom-Left
    drawBitmapRect(image, scene2d::RectF::fromLTRB(0, src_resolution.height - src_center.bottom,
                                                   src_center.left, src_resolution.height),
                   scene2d::RectF::fromLTRB(dst_rect.left, dst_rect.bottom - src_center.bottom,
                                            dst_rect.left + src_center.left, dst_rect.bottom));
    // Bottom
    drawBitmapRect(image, scene2d::RectF::fromLTRB(src_center.left, src_resolution.height - src_center.bottom,
                                                   src_resolution.width - src_center.right, src_resolution.height),
                   scene2d::RectF::fromLTRB(dst_rect.left + src_center.left, dst_rect.bottom - src_center.bottom,
                                            dst_rect.right - src_center.right, dst_rect.bottom));
    // Bottom-Right
    drawBitmapRect(
        image, scene2d::RectF::fromLTRB(src_resolution.width - src_center.right,
                                        src_resolution.height - src_center.bottom,
                                        src_resolution.width, src_resolution.height),
        scene2d::RectF::fromLTRB(dst_rect.right - src_center.right, dst_rect.bottom - src_center.bottom,
                                 dst_rect.right, dst_rect.bottom));
}
}
