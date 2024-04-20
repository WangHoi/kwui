#pragma once

#include "scene2d/Control.h"
#include "scene2d/geom_types.h"
#include "style/StyleColor.h"

namespace graph2d {
class BitmapInterface;
}

namespace scene2d {

class ImageControl : public scene2d::Control {
public:
    static const char* CONTROL_NAME;
    ImageControl();
    base::string_atom name() override;
    void onPaint(graph2d::PainterInterface& p, const scene2d::RectF& rect) override;
    void onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value) override;
    void setImageSource(const std::string& name);

private:
    std::string image_src_;
    std::shared_ptr<graph2d::BitmapInterface> bitmap_;
};

}
