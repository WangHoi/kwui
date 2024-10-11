#pragma once
#include "api/kwui/Application.h"
#include "scene2d/Control.h"
#include "scene2d/geom_types.h"
#include "scene2d/TextLayout.h"
#include "style/StyleColor.h"
#ifdef _WIN32
#include "windows/graphics/PainterD2D.h"
#endif
#include <string>
#include <deque>
#include <memory>
#include <functional>
#if WITH_SKIA
#include "xskia/NativeGLStateX.h"
#endif

namespace scene2d::control
{
class CustomElementControl : public Control, public ::kwui::CustomElementPaintContextInterface
{
public:
    explicit CustomElementControl(base::string_atom name);
    base::string_atom name() override;

    // Implements `Control`
    bool hitTest(const scene2d::PointF& pos, int flags) const override;
    void onAttach(scene2d::Node* node) override;
    void onDetach(Node* node) override;
    void onAnimationFrame(Node* node, absl::Time timestamp) override;
    void onPaint(graph2d::PaintContextInterface& p, const scene2d::RectF& rect) override;
    void onMouseEvent(Node* node, MouseEvent& evt) override;

    // Implements `CustomElementPaintContext`
    void setFillBitmap(void* native_bitmap, float dpi_scale, void (*release_fn)(void*), void* release_udata) override;
    void drawRoundedRect(float left, float top, float width, float height, float radius) override;
    void drawPath(const kwui::CustomElementPaintPath& path, const kwui::CustomElementPaintBrush& brush) override;
    void drawText(const std::string& text, float x, float y,
                  const kwui::CustomElementPaintBrush& brush,
                  const kwui::CustomElementPaintFont& font) override;
    float getDpiScale() const override;
    void writePixels(const void* pixels, size_t src_width, size_t src_height, size_t src_stride, float dst_x,
                     float dst_y, kwui::ColorType color_type) override;
    void beginNativeRendering() override;
    void endNativeRendering() override;

private:
    base::string_atom name_;
    std::unique_ptr<kwui::CustomElement> custom_ = nullptr;
#if _WIN32
    ComPtr<ID2D1Bitmap1> bitmap_;
#endif
    graph2d::PaintContextInterface* cur_painter_ = nullptr;
    RectF cur_rect_;
    std::shared_ptr<graph2d::BitmapInterface> tmp_fill_bitmap_;
#if WITH_SKIA
    std::unique_ptr<xskia::ScopedNativeGLStateX> gl_state_saver_;
#endif
};

ControlFactoryFn CustomElementContrlFactory(base::string_atom name, kwui::CustomElementFactoryFn factory_fn);
}
