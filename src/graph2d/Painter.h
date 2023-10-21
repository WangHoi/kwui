#pragma once
#include "scene2d/geom_types.h"
#include "style/StyleValue.h"

namespace scene2d {
class Control;
}

namespace graph2d {

class TextLayoutInterface;
class GlyphRunInterface;

class PainterInterface {
public:
	virtual void save() = 0;
	virtual void restore() = 0;
	virtual void setTranslation(const scene2d::PointF& offset, bool combine) = 0;
	virtual void drawBox(const scene2d::RectF& rect,
		float border_width,
		const style::Value& background_color,
		const style::Value& border_color) = 0;
	virtual void drawTextLayout(const scene2d::PointF& pos,
		const TextLayoutInterface* text_layout,
		const style::Value& color) = 0;
	virtual void drawGlyphRun(const scene2d::PointF& pos,
		const GlyphRunInterface* text_flow,
		const style::Value& color) = 0;
	virtual void drawControl(const scene2d::RectF& rect,
		scene2d::Control* control) = 0;
};
}