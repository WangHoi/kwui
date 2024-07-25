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
class NativeViewControl : public Control
{
public:
    static const char* CONTROL_NAME;
    NativeViewControl();
    base::string_atom name() override;

    void onAttach(scene2d::Node *node) override;
    void onAnimationFrame(Node* node, absl::Time timestamp) override;
    void onPaint(graph2d::PainterInterface& p, const scene2d::RectF& rect) override;

    static void setNativeViewHandler(kwui::NativeViewHandler* handler);

private:
    windows::graphics::NativeBitmap native_;
    ComPtr<ID2D1Bitmap1> bitmap_;
};
}
