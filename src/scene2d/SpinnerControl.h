#pragma once

#include "scene2d/Control.h"
#include "scene2d/geom_types.h"
#include "style/StyleColor.h"

namespace scene2d {

class SpinnerControl : public scene2d::Control {
public:
    static const char* CONTROL_NAME;
    SpinnerControl();
    base::string_atom name() override;
    void onAttach(scene2d::Node* node) override;
    void onAnimationFrame(scene2d::Node* node, absl::Time timestamp) override;
    void onPaint(graph2d::PainterInterface& p, const scene2d::RectF& rect) override;
    void onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value) override;
    void setBackgroundColor(const style::Color& color);
    void setForegroundColor(const style::Color& color);

private:
    absl::optional<absl::Time> timestamp_;
    style::Color bg_color_;
    style::Color fg_color_;
};

}
