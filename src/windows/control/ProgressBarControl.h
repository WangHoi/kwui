#pragma once
#include "scene2d/Control.h"
#include "scene2d/geom_types.h"
#include "windows/graphics/Color.h"
#include "windows/graphics/TextLayout.h"

namespace windows {
namespace control {

class ProgressBarControl : public scene2d::Control {
public:
    ProgressBarControl();
    base::string_atom name() override;
    void onPaint(graphics::Painter& p, const scene2d::RectF& rect) override;
    void onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value) override;
    inline float GetProgress() const { return _progress; }
    void SetProgress(float value);
    inline void SetColor(const graphics::Color& c) { _color = c; }
    inline void SetBackgroundColor(const graphics::Color& c) { _bg_color = c; }
    void SetBorderRadius(float r) { _border_radius = r; }

private:
    float _progress;
    graphics::Color _bg_color;
    graphics::Color _color;
    float _border_radius;
};

void register_progress_bar_control();

}
}
