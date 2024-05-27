#pragma once
#include "scene2d/Control.h"
#include "scene2d/geom_types.h"
#include "style/StyleColor.h"
#include "windows/graphics/TextLayoutD2D.h"

namespace windows {
namespace control {

class ScrollbarControl : public scene2d::Control {
public:
    enum Orientation {
        VERTICAL,
        HORIZONTAL,
    };
    static const char* CONTROL_NAME;
    ScrollbarControl();
    base::string_atom name() override;
    void onPaint(graph2d::PainterInterface& p, const scene2d::RectF& rect) override;
    void onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value) override;
    
    inline Orientation orientation() const { return orient_; }
    inline void setOrientation(Orientation orient) { orient_ = orient; }
    float scrollLength() const { return scroll_len_; }
    void setScrollLength(float len) { scroll_len_ = len; }
    float clientLength() const { return client_len_; }
    void setClientLength(float len) { client_len_ = len; }
    float scrollOffset() const { return scroll_offset_; }
    void setScrollOffset(float offset) { scroll_offset_ = offset; }

    inline void setColor(const style::Color& c) { color_ = c; }
    inline void setBackgroundColor(const style::Color& c) { bg_color_ = c; }
    void setBorderRadius(float r) { border_radius_ = r; }

private:
    Orientation orient_ = VERTICAL;
    float scroll_len_ = 0.0f;
    float client_len_ = 0.0f;
    float scroll_offset_ = 0.0f;
    style::Color bg_color_;
    style::Color color_;
    float border_radius_;
};

}
}
