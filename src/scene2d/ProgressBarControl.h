#pragma once
#include "scene2d/Control.h"
#include "scene2d/geom_types.h"
#include "style/StyleColor.h"

namespace scene2d {

class ProgressBarControl : public scene2d::Control {
public:
    static const char* CONTROL_NAME;
    ProgressBarControl();
    base::string_atom name() override;
    void onPaint(graph2d::PaintContextInterface& p, const scene2d::RectF& rect) override;
    void onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value) override;
    inline float GetProgress() const { return progress_; }
    void SetProgress(float value);
    inline void SetColor(const style::Color& c) { fg_color_ = c; }
    inline void SetBackgroundColor(const style::Color& c) { bg_color_ = c; }
    void SetBorderRadius(float r) { border_radius_ = r; }

private:
    float progress_;
    style::Color bg_color_;
    style::Color fg_color_;
    float border_radius_;
};

}
