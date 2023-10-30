#pragma once
#include "scene2d/Control.h"
#include "scene2d/geom_types.h"
#include "windows/graphics/Color.h"
#include "windows/graphics/TextLayout.h"

namespace windows {
namespace control {

class ScrollBarControl : public scene2d::Control {
public:
    enum Orientation {
        VERTICAL,
        HORIZONTAL,
    };
    static const char* CONTROL_NAME;
    ScrollBarControl();
    base::string_atom name() override;
    void onPaint(graph2d::PainterInterface& p, const scene2d::RectF& rect) override;
    void onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value) override;
    
    inline Orientation orientation() const { return orient_; }
    inline void setOrientation(Orientation orient) { orient_ = orient; }
    float innerLength() const { return inner_len_; }
    void setInnerLength(float len) { inner_len_ = len; }
    float viewportLength() const { return viewport_len_; }
    void setViewportLength(float len) { viewport_len_ = len; }
    float scrollOffset() const { return scroll_offset_; }
    void setScrollOffset(float offset) { scroll_offset_ = offset; }

    inline void setColor(const graphics::Color& c) { color_ = c; }
    inline void setBackgroundColor(const graphics::Color& c) { bg_color_ = c; }
    void setBorderRadius(float r) { border_radius_ = r; }

private:
    Orientation orient_ = VERTICAL;
    float inner_len_ = 0.0f;
    float viewport_len_ = 0.0f;
    float scroll_offset_ = 0.0f;
    graphics::Color bg_color_;
    graphics::Color color_;
    float border_radius_;
};

}
}
