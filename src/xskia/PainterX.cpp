#include "PainterX.h"
#include "TextFlowX.h"
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
			SkSamplingOptions());
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
	const graph2d::GlyphRunInterface* text_flow,
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
	// control->onPaint(*this, rect);
}

}