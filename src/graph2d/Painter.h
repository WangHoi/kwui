#pragma once
#include "scene2d/geom_types.h"
#include "style/StyleValue.h"
#include "style/StyleColor.h"

namespace scene2d {
class Control;
class TextLayoutInterface;
}
namespace style {
struct EdgeOffsetF;
struct CornerRadiusF;
class GlyphRunInterface;
}

namespace graph2d {

class GlyphRunInterface;
class BitmapInterface;

class PainterInterface {
public:
	virtual ~PainterInterface() = default;
	virtual void save() = 0;
	virtual void restore() = 0;
	virtual float getDpiScale() = 0;
	virtual void setTranslation(const scene2d::PointF& offset, bool combine) = 0;
	virtual void setRotation(float degrees, const scene2d::PointF& center, bool combine) = 0;
	virtual void pushClipRect(const scene2d::PointF& origin, const scene2d::DimensionF& size) = 0;
	virtual void popClipRect() = 0;
	virtual void clear(const style::Color& c) = 0;
	virtual void drawBox(const scene2d::RectF& padding_rect,
		const style::EdgeOffsetF& inset_border_width,
		const style::CornerRadiusF& border_radius,
		const style::Color& background_color,
		const style::Color& border_color,
		const BitmapInterface* background_image = nullptr) = 0;
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
		const style::CornerRadiusF& border_radius,
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