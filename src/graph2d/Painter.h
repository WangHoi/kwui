#pragma once
#include "scene2d/geom_types.h"
#include "style/StyleValue.h"
#include "style/StyleColor.h"

namespace scene2d {
class Control;
}
namespace style {
struct EdgeOffsetF;
struct CornerRadiusF;
}

namespace graph2d {

class GlyphRunInterface;
class BitmapInterface;

class PainterInterface {
public:
	virtual ~PainterInterface() = default;
	virtual void save() = 0;
	virtual void restore() = 0;
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
		const GlyphRunInterface* text_flow,
		const style::Color& color) = 0;
	virtual void drawControl(const scene2d::RectF& rect,
		scene2d::Control* control) = 0;
};
}