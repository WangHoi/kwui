#pragma once

#include "scene2d/Control.h"
#include "scene2d/geom_types.h"
#include "style/StyleColor.h"
#include "windows/windows_header.h"

namespace windows {
namespace control {

class ImageButtonControl : public scene2d::Control {
public:
    static const char* CONTROL_NAME;
    ImageButtonControl();
    base::string_atom name() override;
    bool hitTest(const scene2d::PointF& pos, int flags) const override;
    void onPaint(graph2d::PainterInterface& p, const scene2d::RectF& rect) override;
    void onMouseEvent(scene2d::Node* node, scene2d::MouseEvent& evt) override;
    void onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value) override;
    void onSetEventHandler(base::string_atom name, JSValue func) override;

private:
    std::string _src;
    std::string _hover_src;
    ComPtr<ID2D1Bitmap> _bitmap;
    ComPtr<ID2D1Bitmap> _hover_bitmap;
    JSValue onclick_func_ = JS_UNINITIALIZED;
};

}
}
