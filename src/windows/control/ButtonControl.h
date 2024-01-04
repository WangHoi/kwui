#pragma once

#include "scene2d/Control.h"
#include "scene2d/geom_types.h"
#include "style/StyleColor.h"
#include "windows/windows_header.h"

namespace windows {
namespace control {

class ButtonControl : public scene2d::Control {
public:
    static const char* CONTROL_NAME;
    ButtonControl();
    ~ButtonControl();
    base::string_atom name() override;
    bool hitTest(const scene2d::PointF& pos, int flags) const override;
    void onPaint(graph2d::PainterInterface& p, const scene2d::RectF& rect) override;
    void onMouseEvent(scene2d::Node* node, scene2d::MouseEvent& evt) override;
    void onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value) override;
    void onSetEventHandler(base::string_atom name, const script::Value& func) override;
    void onDetach(scene2d::Node* node) override;

private:
    style::Color bg_color_;
    style::Color bg_hover_color_;
    script::Value onclick_func_;
};

}
}
