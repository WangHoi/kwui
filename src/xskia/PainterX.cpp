#include "PainterX.h"
#include "TextFlowX.h"
#include "TextLayoutX.h"
#include "BitmapX.h"
#include "style/Layout.h"
#include "scene2d/Control.h"
#include "include/core/SkRRect.h"
#include "base/log.h"

namespace xskia {

PainterX::PainterX(SkCanvas* canvas, float dpi_scale)
	: canvas_(canvas), dpi_scale_(dpi_scale)
{}

void PainterX::save()
{
	canvas_->save();
}

void PainterX::restore()
{
	canvas_->restore();
}

void PainterX::setTranslation(const scene2d::PointF& offset, bool combine)
{
	canvas_->translate(offset.x, offset.y);
}

void PainterX::setRotation(float degrees, const scene2d::PointF& center, bool combine)
{
	canvas_->rotate(degrees, center.x, center.y);
}

void PainterX::pushClipRect(const scene2d::PointF& origin, const scene2d::DimensionF& size)
{
	canvas_->save();
	canvas_->clipIRect(SkIRect::MakeLTRB(floor(origin.x), floor(origin.y),
		ceil(origin.x + size.width), ceil(origin.y + size.height)));
}

void PainterX::popClipRect()
{
	canvas_->restore();
}
void PainterX::clear(const style::Color& c)
{
	canvas_->clear(c);
}
void PainterX::drawBox(
	const scene2d::RectF& padding_rect,
	const style::EdgeOffsetF& border_width,
	const style::CornerRadiusF& border_radius,
	const style::Color& background_color,
	const style::Color& border_color,
	const graph2d::BitmapInterface* background_image)
{
	SkPaint paint;
	SkRRect rrect;
	SkVector radii[4] = {
		SkVector::Make(border_radius.top_left, border_radius.top_left),
		SkVector::Make(border_radius.top_right, border_radius.top_right),
		SkVector::Make(border_radius.bottom_right, border_radius.bottom_right),
		SkVector::Make(border_radius.bottom_left, border_radius.bottom_left),
	};
	rrect.setRectRadii(padding_rect, radii);
	float max_border_width = std::max(
		{ border_width.left, border_width.top, border_width.right, border_width.bottom });
	rrect.outset(max_border_width * 0.5f, max_border_width * 0.5f);
	paint.setColor(background_color);
	canvas_->drawRRect(rrect, paint);

	auto image = (const BitmapX*)background_image;
	if (image && image->skImage()) {
		canvas_->drawImageRect(image->skImage(),
			padding_rect,
			SkSamplingOptions(SkFilterMode::kLinear));
	}

	if (max_border_width > 0.0f) {
		paint.setStroke(true);
		paint.setStrokeWidth(max_border_width);
		paint.setColor(border_color);
		canvas_->drawRRect(rrect, paint);
	}
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
		SkPoint{ pos.x, pos.y },
		run->font_,
		paint);
}

void PainterX::drawControl(const scene2d::RectF& rect, scene2d::Control* control)
{
	control->onPaint(*this, rect);
}

void PainterX::drawBitmap(const graph2d::BitmapInterface* image, const scene2d::PointF& origin, const scene2d::DimensionF& size)
{
	auto bitmap = static_cast<const BitmapX*>(image)->skImage();
	if (bitmap) {
		canvas_->drawImageRect(bitmap,
			SkRect::MakeXYWH(origin.x, origin.y, size.width, size.height),
			SkSamplingOptions());
	}
}

void PainterX::drawRoundedRect(const scene2d::RectF& rect,
	const style::CornerRadiusF& border_radius,
	const style::Color& background_color)
{
	SkPaint paint;
	SkRRect rrect;
	SkVector radii[4] = {
		SkVector::Make(border_radius.top_left, border_radius.top_left),
		SkVector::Make(border_radius.top_right, border_radius.top_right),
		SkVector::Make(border_radius.bottom_right, border_radius.bottom_right),
		SkVector::Make(border_radius.bottom_left, border_radius.bottom_left),
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

}