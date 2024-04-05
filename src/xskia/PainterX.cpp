#include "PainterX.h"
#include "TextFlowX.h"

namespace xskia {

PainterX::PainterX(SkCanvas* canvas)
	: canvas_(canvas)
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
}

void PainterX::popClipRect()
{
}
void PainterX::clear(const style::Color& c)
{
	canvas_->clear(c);
}
void PainterX::drawBox(
	const scene2d::RectF& border_rect,
	const style::EdgeOffsetF& inset_border_width,
	const style::CornerRadiusF& border_radius,
	const style::Color& background_color,
	const style::Color& border_color,
	const graph2d::BitmapInterface* background_image)
{
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
}

}