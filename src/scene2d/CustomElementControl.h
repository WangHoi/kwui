#pragma once
#include "api/kwui/Application.h"
#include "scene2d/Control.h"
#include "scene2d/geom_types.h"
#include "scene2d/TextLayout.h"
#include "style/StyleColor.h"
#include "windows/graphics/PainterD2D.h"
#include <string>
#include <deque>
#include <memory>
#include <functional>

namespace scene2d::control
{
class CustomElementControl : public Control, public ::kwui::CustomElementPaintContextInterface
{
public:
    explicit CustomElementControl(base::string_atom name);
    base::string_atom name() override;

    // Implements `Control`
    void onAttach(scene2d::Node* node) override;
    void onDetach(Node* node) override;
    void onAnimationFrame(Node* node, absl::Time timestamp) override;
    void onPaint(graph2d::PaintContextInterface& p, const scene2d::RectF& rect) override;

    // Implements `CustomElementPaintContext`
    void* getNativeBitmap(float& out_pixel_width, float& out_pixel_height) override;
    void setFillBitmap(void* native_bitmap) override;
    void drawRoundedRect(float left, float top, float width, float height, float radius) override;
    void drawPath(const kwui::CustomElementPaintPath* path) override;

private:
    base::string_atom name_;
    std::unique_ptr<kwui::CustomElement> custom_ = nullptr;
    windows::graphics::NativeBitmap native_;
    ComPtr<ID2D1Bitmap1> bitmap_;
    graph2d::PaintContextInterface* cur_painter_ = nullptr;
    RectF cur_rect_;
};

ControlFactoryFn CustomElementContrlFactory(base::string_atom name, kwui::CustomElementFactoryFn factory_fn);

}
