#pragma once

#include "Control.h"
#include "geom_types.h"
#include "style/StyleColor.h"

namespace scene2d {

class ButtonControl : public scene2d::Control {
public:
    static const char* CONTROL_NAME;
    ButtonControl();
    ~ButtonControl();
    base::string_atom name() override;
    bool hitTest(const scene2d::PointF& pos, int flags) const override;
    void onPaint(graph2d::PaintContextInterface& p, const scene2d::RectF& rect) override;
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
