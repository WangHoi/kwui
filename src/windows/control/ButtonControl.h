#pragma once

#include "scene2d/Control.h"
#include "scene2d/geom_types.h"
#include "windows/graphics/Color.h"
#include "windows/windows_header.h"

namespace windows {
namespace control {

class ButtonControl : public scene2d::Control {
public:
    static const char* CONTROL_NAME;
    ButtonControl();
    base::string_atom name() override;
    bool testFlags(int flags) const override;
    void onPaint(graphics::Painter& p, const scene2d::RectF& rect) override;
    void onMouseEvent(scene2d::Node* node, scene2d::MouseEvent& evt) override;
    void onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value) override;
    void onSetEventHandler(base::string_atom name, JSValue func) override;

private:
    graphics::Color bg_color_;
    graphics::Color bg_hover_color_;
    JSValue onclick_func_ = JS_UNINITIALIZED;
};

}
}
