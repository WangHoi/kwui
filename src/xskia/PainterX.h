#include <include/gpu/GrBackendSurface.h>

#include "graph2d/PaintContextInterface.h"
#include "include/core/SkCanvas.h"

namespace xskia
{
class BitmapX;
class PaintSurfaceX;

class PainterX : public graph2d::PaintContextInterface
{
public:
    PainterX(PaintSurfaceX* surface, SkCanvas* canvas, float dpi_scale);

    // 通过 PainterInterface 继承
    void save() override;
    void restore() override;
    float getDpiScale() override;
    void translate(const scene2d::PointF& offset) override;
    // void rotate(float radians, const scene2d::PointF& center) override;
    void clipRect(const scene2d::RectF& rect) override;
    void clear(const style::Color& c) override;
    void drawGlyphRun(const scene2d::PointF& pos, const style::GlyphRunInterface* text_flow,
                      const style::Color& color) override;
    void drawControl(const scene2d::RectF& rect, scene2d::Control* control) override;
    void drawBitmap(const graph2d::BitmapInterface* image,
                    const scene2d::PointF& origin,
                    const scene2d::DimensionF& size) override;
    void drawRoundedRect(const scene2d::RectF& rect,
                         const scene2d::CornerRadiusF& border_radius,
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
    void drawRect(const scene2d::RectF& rect, const graph2d::PaintBrush& brush) override;
    void drawRRect(const scene2d::RRectF& rrect, const graph2d::PaintBrush& brush) override;
    void drawDRRect(const scene2d::RRectF& outer, const scene2d::RRectF& inner,
                    const graph2d::PaintBrush& brush) override;
    void drawPath(const graph2d::PaintPathInterface* path, const graph2d::PaintBrush& brush) override;
    void drawBoxShadow(const scene2d::RectF& padding_rect, const style::EdgeOffsetF& inset_border_width,
                       const scene2d::CornerRadiusF& border_radius, const graph2d::BoxShadow& box_shadow) override;
    void drawBitmapRect(const graph2d::BitmapInterface* image, const scene2d::RectF& src_rect,
                        const scene2d::RectF& dst_rect) override;
    std::shared_ptr<graph2d::BitmapInterface> adoptBackendTexture(const GrBackendTexture& tex);

private:
    void flattenSkPaint(absl::FunctionRef<void(const SkPaint&)> func, const graph2d::PaintBrush& brush, const scene2d::PointF* offset = nullptr) const;
    // returns a 1.0 dpi image
    sk_sp<SkImage> makeOutsetShadowBitmap(const scene2d::RectF& padding_rect,
                                          const style::EdgeOffsetF& inset_border_width,
                                          const scene2d::CornerRadiusF& border_radius,
                                          const graph2d::BoxShadow& box_shadow,
                                          absl::optional<style::EdgeOffsetF>& expand_edges);
    // returns a 1.0 dpi image, with extra 4 pixel padding around
    sk_sp<SkImage> makeInsetShadowBitmap(const scene2d::RectF& padding_rect,
                                         const style::EdgeOffsetF& inset_border_width,
                                         const scene2d::CornerRadiusF& border_radius,
                                         const graph2d::BoxShadow& box_shadow);

    PaintSurfaceX* surface_;
    SkCanvas* canvas_;
    float dpi_scale_;
};
}
