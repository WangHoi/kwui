#include "graph2d/Painter.h"
#include "include/core/SkCanvas.h"

namespace xskia {

class PainterX : public graph2d::PainterInterface {
public:
	PainterX(SkCanvas* canvas, float dpi_scale);

	// 通过 PainterInterface 继承
	void save() override;
	void restore() override;
	float getDpiScale() override;
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
	void drawGlyphRun(const scene2d::PointF& pos, const style::GlyphRunInterface* text_flow, const style::Color& color) override;
	void drawControl(const scene2d::RectF& rect, scene2d::Control* control) override;
	void drawBitmap(const graph2d::BitmapInterface* image,
		const scene2d::PointF& origin,
		const scene2d::DimensionF& size) override;
	void drawRoundedRect(const scene2d::RectF& rect,
		const style::CornerRadiusF& border_radius,
		const style::Color& background_color) override;
	void drawRect(const scene2d::RectF& rect,
		const style::Color& background_color) override;
	void drawTextLayout(const scene2d::PointF& origin,
		const scene2d::TextLayoutInterface& layout,
		const style::Color& color) override;
	void drawCircle(const scene2d::PointF& center,
		float radius,
		const style::Color& background_color,
		float border_width,
		const style::Color& border_color) override;
	void drawArc(const scene2d::PointF& center,
		float radius,
		float start_angle,
		float span_angle,
		const style::Color& background_color,
		float border_width,
		const style::Color& border_color) override;

private:
	SkCanvas* canvas_;
	float dpi_scale_;
};

}