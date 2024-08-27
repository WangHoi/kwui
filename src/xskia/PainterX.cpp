#include "PainterX.h"

#include <include/core/SkData.h>
#include <src/shaders/SkImageShader.h>

#include "TextFlowX.h"
#include "TextLayoutX.h"
#include "BitmapX.h"
#include "PaintPathX.h"
#include "PaintSurfaceX.h"
#include "style/Layout.h"
#include "scene2d/Control.h"
#include "include/core/SkRRect.h"
#include "base/log.h"
#include "graph2d/image_blur.h"

namespace xskia
{
PainterX::PainterX(PaintSurfaceX* surface, SkCanvas* canvas, float dpi_scale)
    : surface_(surface), canvas_(canvas), dpi_scale_(dpi_scale)
{
}

void PainterX::save()
{
    canvas_->save();
}

void PainterX::restore()
{
    canvas_->restore();
}

float PainterX::getDpiScale()
{
    return dpi_scale_;
}

void PainterX::translate(const scene2d::PointF& offset)
{
    canvas_->translate(offset.x, offset.y);
}

// void PainterX::rotate(float radians, const scene2d::PointF& center)
// {
// 	canvas_->rotate(radians, center.x, center.y);
// }

void PainterX::clipRect(const scene2d::RectF& rect)
{
    canvas_->save();
    canvas_->clipIRect(SkIRect::MakeLTRB(floor(rect.left), floor(rect.top),
                                         ceil(rect.right), ceil(rect.bottom)));
}

void PainterX::clear(const style::Color& c)
{
    canvas_->clear(c);
}

void PainterX::drawGlyphRun(
    const scene2d::PointF& pos,
    const style::GlyphRunInterface* text_flow,
    const style::Color& color)
{
    SkPaint paint;
    paint.setColor(color);
    auto run = static_cast<const GlyphRunX*>(text_flow);
    canvas_->drawGlyphs((int)run->glyphs_.size(),
                        run->glyphs_.data(),
                        run->positions_.data(),
                        SkPoint{pos.x, pos.y},
                        run->font_,
                        paint);
}

void PainterX::drawControl(const scene2d::RectF& rect, scene2d::Control* control)
{
    control->onPaint(*this, rect);
}

void PainterX::drawBitmap(const graph2d::BitmapInterface* image, const scene2d::PointF& origin,
                          const scene2d::DimensionF& size)
{
    auto bitmap = static_cast<const BitmapXInterface*>(image)->skImage();
    if (bitmap) {
        canvas_->drawImageRect(bitmap,
                               SkRect::MakeXYWH(origin.x, origin.y, size.width, size.height),
                               SkSamplingOptions());
    }
}

void PainterX::drawRoundedRect(const scene2d::RectF& rect,
                               const scene2d::CornerRadiusF& border_radius,
                               const style::Color& background_color)
{
    SkPaint paint;
    SkRRect rrect;
    SkVector radii[4] = {
        SkVector::Make(border_radius.top_left.width, border_radius.top_left.height),
        SkVector::Make(border_radius.top_right.width, border_radius.top_right.height),
        SkVector::Make(border_radius.bottom_right.width, border_radius.bottom_right.height),
        SkVector::Make(border_radius.bottom_left.width, border_radius.bottom_left.height),
    };
    rrect.setRectRadii(rect, radii);
    paint.setColor(background_color);
    canvas_->drawRRect(rrect, paint);
}

void PainterX::drawRect(const scene2d::RectF& rect,
                        const style::Color& background_color)
{
    SkPaint paint;
    paint.setColor(background_color);
    canvas_->drawRect(rect, paint);
}

void PainterX::drawTextLayout(const scene2d::PointF& origin,
                              const scene2d::TextLayoutInterface& layout,
                              const style::Color& color)
{
    const xskia::TextLayoutX& l = (const xskia::TextLayoutX&)layout;
    scene2d::PointF baseline_origin(origin.x, origin.y + layout.baseline());
    drawGlyphRun(baseline_origin, l.glyphRun(), color);
}

void PainterX::drawCircle(const scene2d::PointF& center,
                          float radius,
                          const style::Color& background_color,
                          float border_width,
                          const style::Color& border_color)
{
    SkPaint paint;
    paint.setColor(background_color);
    canvas_->drawCircle(center, radius, paint);

    if (border_width > 0.0f) {
        paint.setStroke(true);
        paint.setStrokeWidth(border_width);
        paint.setColor(border_color);
        canvas_->drawCircle(center, radius, paint);
    }
}

void PainterX::drawArc(const scene2d::PointF& center,
                       float radius,
                       float start_angle,
                       float span_angle,
                       const style::Color& background_color,
                       float border_width,
                       const style::Color& border_color)
{
    SkPaint paint;
    SkRect oval;
    paint.setColor(background_color);
    oval.setLTRB(center.x - radius,
                 center.y - radius,
                 center.x + radius,
                 center.y + radius);
    canvas_->drawArc(oval, start_angle, span_angle, false, paint);

    if (border_width > 0.0f) {
        paint.setStroke(true);
        paint.setStrokeWidth(border_width);
        paint.setColor(border_color);
        canvas_->drawArc(oval, start_angle, span_angle, false, paint);
    }
}

void PainterX::drawRect(const scene2d::RectF& rect, const graph2d::PaintBrush& brush)
{
    canvas_->drawRect(rect, makeSkPaint(brush));
}

void PainterX::drawRRect(const scene2d::RRectF& rrect, const graph2d::PaintBrush& brush)
{
    canvas_->drawRRect(rrect, makeSkPaint(brush));
}

void PainterX::drawDRRect(const scene2d::RRectF& outer, const scene2d::RRectF& inner, const graph2d::PaintBrush& brush)
{
    canvas_->drawDRRect(outer, inner, makeSkPaint(brush));
}

void PainterX::drawPath(const graph2d::PaintPathInterface* path, const graph2d::PaintBrush& brush)
{
    auto sk_path = static_cast<const PaintPathX*>(path);
    canvas_->drawPath(sk_path->skPath(), makeSkPaint(brush));
}

void PainterX::drawBoxShadow(const scene2d::RectF& padding_rect, const style::EdgeOffsetF& inset_border_width,
                             const scene2d::CornerRadiusF& border_radius, const graph2d::BoxShadow& box_shadow)
{
    if (box_shadow.inset) {
        // Compute extra padding
        float blur_radius = roundf(box_shadow.blur_radius * dpi_scale_) / dpi_scale_;
        auto rect = scene2d::RectF::fromXYWH(blur_radius,
                                             blur_radius,
                                             padding_rect.width() + inset_border_width.left +
                                             inset_border_width.right,
                                             padding_rect.height() + inset_border_width.top +
                                             inset_border_width.bottom);

        // Blit the opacity bitmap
        auto dst_rrect = scene2d::RRectF::fromRectRadius(padding_rect, border_radius);
        auto src_origin = scene2d::PointF(
            padding_rect.left - inset_border_width.left - blur_radius + box_shadow.offset_x,
            padding_rect.top - inset_border_width.top - blur_radius + box_shadow.offset_y);

        // Create shadow bitmap
        auto shadow_bmp = makeInsetShadowBitmap(padding_rect,
                                                inset_border_width,
                                                border_radius,
                                                box_shadow);
        SkPaint paint;
        paint.setColor(box_shadow.color);
        SkMatrix paint_scale = SkMatrix::Scale(1.0f / dpi_scale_, 1.0f / dpi_scale_);
        paint.setShader(SkImageShader::Make(shadow_bmp, SkTileMode::kClamp, SkTileMode::kClamp,
                                            SkSamplingOptions(SkFilterMode::kLinear), &paint_scale));

        // Draw the RRect
        SkPath path;
        path.addRRect(dst_rrect);
        // canvas_->drawPath(path, paint);
    } else {
        // Compute extra padding
        float blur_radius = roundf(box_shadow.blur_radius * dpi_scale_) / dpi_scale_;
        auto rect = scene2d::RectF::fromXYWH(blur_radius,
                                             blur_radius,
                                             padding_rect.width() + inset_border_width.left +
                                             inset_border_width.right,
                                             padding_rect.height() + inset_border_width.top +
                                             inset_border_width.bottom);
        scene2d::RRectF rrect = scene2d::RRectF::fromRectRadius(
            scene2d::RectF::fromLTRB(rect.left, rect.top, rect.right, rect.bottom),
            border_radius);

        // Blit the opacity bitmap
        auto dst_rect = scene2d::RectF::fromXYWH(
            padding_rect.left - inset_border_width.left - blur_radius + box_shadow.offset_x,
            padding_rect.top - inset_border_width.top - blur_radius + box_shadow.offset_y,
            blur_radius + rrect.width() + blur_radius,
            blur_radius + rrect.height() + blur_radius);

        absl::optional<style::EdgeOffsetF> expand_edges;
        auto shadow_bmp = makeOutsetShadowBitmap(padding_rect,
                                                 inset_border_width,
                                                 border_radius,
                                                 box_shadow,
                                                 expand_edges);
        if (expand_edges.has_value()) {
            BitmapX bmp(shadow_bmp);
            drawBitmapNine(&bmp, expand_edges.value(), dst_rect);
        } else {
            canvas_->drawImageRect(shadow_bmp.get(), dst_rect, SkSamplingOptions(SkFilterMode::kLinear));
        }
    }
}

void PainterX::drawBitmapRect(const graph2d::BitmapInterface* image, const scene2d::RectF& src_rect,
                              const scene2d::RectF& dst_rect)
{
    // TODO: handle source bitmap and dst render surface DPI mismatch.
    auto img = static_cast<const BitmapXInterface*>(image);
    canvas_->drawImageRect(img->skImage().get(), src_rect, dst_rect,
                           SkSamplingOptions(SkFilterMode::kLinear),
                           nullptr,
                           SkCanvas::kStrict_SrcRectConstraint);
}

SkPaint PainterX::makeSkPaint(const graph2d::PaintBrush& brush) const
{
    SkPaint p;
    p.setAntiAlias(true);
    p.setColor(brush.color());
    if (auto img = brush.shader()) {
        p.setShader(SkImageShader::Make(static_cast<const BitmapFromUrlX*>(img)->skImage(), SkTileMode::kClamp,
                                        SkTileMode::kClamp, SkSamplingOptions(), nullptr));
    }
    if (brush.style() == graph2d::PAINT_STYLE_STROKE) {
        p.setStroke(true);
        p.setStrokeWidth(brush.strokeWidth());
        switch (brush.strokeCap()) {
        case graph2d::STROKE_CAP_BUTT: p.setStrokeCap(SkPaint::kButt_Cap);
            break;
        case graph2d::STROKE_CAP_ROUND: p.setStrokeCap(SkPaint::kRound_Cap);
            break;
        case graph2d::STROKE_CAP_SQUARE: p.setStrokeCap(SkPaint::kSquare_Cap);
            break;
        default: ;
        }
        switch (brush.strokeJoin()) {
        case graph2d::STROKE_JOIN_MITER: p.setStrokeJoin(SkPaint::kMiter_Join);
            break;
        case graph2d::STROKE_JOIN_ROUND: p.setStrokeJoin(SkPaint::kRound_Join);
            break;
        case graph2d::STROKE_JOIN_BEVEL: p.setStrokeJoin(SkPaint::kBevel_Join);
            break;
        default: ;
        }
        p.setStrokeMiter(brush.strokeMiterLimit());
    }
    return p;
}

sk_sp<SkImage> PainterX::makeOutsetShadowBitmap(const scene2d::RectF& padding_rect,
                                                const style::EdgeOffsetF& inset_border_width,
                                                const scene2d::CornerRadiusF& border_radius,
                                                const graph2d::BoxShadow& box_shadow,
                                                absl::optional<style::EdgeOffsetF>& expand_edges)
{
    // Compute extra padding
    const float blur_radius = roundf(box_shadow.blur_radius * dpi_scale_) / dpi_scale_;
    auto rect = scene2d::RectF::fromXYWH(blur_radius,
                                         blur_radius,
                                         padding_rect.width() + inset_border_width.left +
                                         inset_border_width.right,
                                         padding_rect.height() + inset_border_width.top +
                                         inset_border_width.bottom);

    // Check expand
    auto max_border_radius = std::max({
        border_radius.top_left.width + inset_border_width.left,
        border_radius.top_left.height + inset_border_width.top,
        border_radius.top_right.width + inset_border_width.right,
        border_radius.top_right.height + inset_border_width.top,
        border_radius.bottom_right.width + inset_border_width.right,
        border_radius.bottom_right.height + inset_border_width.bottom,
        border_radius.bottom_left.width + inset_border_width.left,
        border_radius.bottom_left.height + inset_border_width.bottom
    });
    if (rect.right - rect.left >= 2 * max_border_radius
        && rect.bottom - rect.top >= 2 * max_border_radius) {
        expand_edges = style::EdgeOffsetF::fromAll(blur_radius + max_border_radius);
        rect = scene2d::RectF::fromXYWH(blur_radius,
                                        blur_radius,
                                        2 * (max_border_radius + 1),
                                        2 * (max_border_radius + 1));
    }

    scene2d::RRectF rrect = scene2d::RRectF::fromRectRadius(
        scene2d::RectF::fromLTRB(rect.left, rect.top, rect.right, rect.bottom),
        border_radius);

    // Check bitmap cache
    std::string cache_key = absl::StrFormat("box-shadow: outset, blur=%.2f, pixel_size=%.0fx%.0f, color=#%08x",
                                            blur_radius,
                                            (blur_radius + rrect.width() + blur_radius) * dpi_scale_,
                                            (blur_radius + rrect.height() + blur_radius) * dpi_scale_,
                                            box_shadow.color.getRgba());

    if (surface_) {
        auto b = surface_->getCachedBitmap(cache_key);
        if (b)
            return b;
    }

    // Create shadow bitmap
    SkImageInfo info = SkImageInfo::MakeN32Premul(ceilf(blur_radius + rrect.width() + blur_radius) * dpi_scale_,
                                                  ceilf(blur_radius + rrect.height() + blur_radius) * dpi_scale_);
    size_t stride = info.minRowBytes();
    size_t size = info.computeByteSize(stride);
    auto pixel_data = SkData::MakeZeroInitialized(size);
    sk_sp<SkSurface> bmp = SkSurface::MakeRasterDirect(info, pixel_data->writable_data(), stride);
    SkCanvas* bp = bmp->getCanvas();
    bp->scale(dpi_scale_, dpi_scale_);
    graph2d::PaintBrush brush;
    brush.setColor(box_shadow.color);
    bp->drawRRect(rrect, makeSkPaint(brush));
    bmp->flush();

    // Blur the bitmap
    JuceImage jimg = JuceImage::fromARGB(info.width(),
                                         info.height(),
                                         pixel_data->writable_data(),
                                         stride);
    applyStackBlur(jimg, blur_radius * dpi_scale_);

    // Make the bitmap
    auto shadow_bitmap = SkImage::MakeRasterData(info, pixel_data, stride);

    if (surface_ && shadow_bitmap)
        surface_->updateCachedBitmap(cache_key, shadow_bitmap);
    return shadow_bitmap;
}

sk_sp<SkImage> PainterX::makeInsetShadowBitmap(const scene2d::RectF& padding_rect,
                                               const style::EdgeOffsetF& inset_border_width,
                                               const scene2d::CornerRadiusF& border_radius,
                                               const graph2d::BoxShadow& box_shadow)
{
    // Compute extra padding
    float blur_radius = roundf(box_shadow.blur_radius * dpi_scale_) / dpi_scale_;
    auto rect = scene2d::RectF::fromXYWH(blur_radius,
                                         blur_radius,
                                         padding_rect.width() + inset_border_width.left +
                                         inset_border_width.right,
                                         padding_rect.height() + inset_border_width.top +
                                         inset_border_width.bottom);
    scene2d::RRectF rrect = scene2d::RRectF::fromRectRadius(
        scene2d::RectF::fromLTRB(rect.left, rect.top, rect.right, rect.bottom),
        border_radius);

    // Check bitmap cache
    std::string cache_key = absl::StrFormat("box-shadow: inset, blur=%.2f, pixel_size=%.0fx%.0f, color=#%08x",
                                            blur_radius,
                                            (blur_radius + rrect.width() + blur_radius) * dpi_scale_,
                                            (blur_radius + rrect.height() + blur_radius) * dpi_scale_,
                                            box_shadow.color.getRgba());

    if (surface_) {
        auto b = surface_->getCachedBitmap(cache_key);
        if (b)
            return b;
    }

    // Create shadow bitmap
    SkImageInfo info = SkImageInfo::MakeN32Premul((blur_radius + rrect.width() + blur_radius) * dpi_scale_,
                                                  (blur_radius + rrect.height() + blur_radius) * dpi_scale_);
    size_t stride = info.minRowBytes();
    size_t size = info.computeByteSize(stride);
    auto pixel_data = SkData::MakeZeroInitialized(size);
    sk_sp<SkSurface> bmp = SkSurface::MakeRasterDirect(info, pixel_data->writable_data(), stride);
    SkCanvas* bp = bmp->getCanvas();
    bp->scale(dpi_scale_, dpi_scale_);

    // Draw the border
    graph2d::PaintBrush brush;
    brush.setColor(box_shadow.color);
    auto outer = scene2d::RRectF::fromRectRadius(
        scene2d::RectF::fromXYWH(0, 0, info.width() / dpi_scale_, info.height() / dpi_scale_),
        scene2d::CornerRadiusF());
    bp->drawDRRect(outer, rrect, makeSkPaint(brush));
    bp->flush();

    // Blur the bitmap
    JuceImage jimg = JuceImage::fromARGB(info.width(),
                                         info.height(),
                                         pixel_data->writable_data(),
                                         stride);
    applyStackBlur(jimg, blur_radius * dpi_scale_);

    // Make the bitmap
    auto shadow_bitmap = SkImage::MakeRasterData(info, pixel_data, stride);
    if (surface_ && shadow_bitmap)
        surface_->updateCachedBitmap(cache_key, shadow_bitmap);
    return shadow_bitmap;
}
}
