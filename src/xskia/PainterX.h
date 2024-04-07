#include "graph2d/Painter.h"
#include "include/core/SkCanvas.h"

namespace xskia {

class PainterX : public graph2d::PainterInterface {
public:
	PainterX(SkCanvas* canvas);

private:
	SkCanvas* canvas_;

	// 通过 PainterInterface 继承
	void save() override;
	void restore() override;
	void setTranslation(const scene2d::PointF& offset, bool combine) override;
	void setRotation(float degrees, const scene2d::PointF& center, bool combine) override;
	void pushClipRect(const scene2d::PointF& origin, const scene2d::DimensionF& size) override;
	void popClipRect() override;
	void clear(const style::Color& c) override;
	void drawBox(const scene2d::RectF& border_rect,
		const style::EdgeOffsetF& inset_border_width,
		const style::CornerRadiusF& border_radius,
		const style::Color& background_color,
		const style::Color& border_color,
		const graph2d::BitmapInterface* background_image) override;
	void drawGlyphRun(const scene2d::PointF& pos, const graph2d::GlyphRunInterface* text_flow, const style::Color& color) override;
	void drawControl(const scene2d::RectF& rect, scene2d::Control* control) override;
};

}