#pragma once

#include "kwui/CustomElement.h"
#include <vector>

#include "Formula.h"
#include "CartesianCoord.h"
#include "ImplicitPlot.h"

class PlotElement : public kwui::CustomElement
{
public:
    PlotElement();
    void onPaint(kwui::CustomElementPaintContextInterface& p, const kwui::CustomElementPaintOption& po) override;

private:
    void drawAxis(kwui::CustomElementPaintContextInterface& p, float offset, float sx, float sy, float ex, float ey);

public:
    void onWheel(float wheel_delta) override;
    void onMouseDown(kwui::ButtonState button, int buttons, float x, float y) override;
    void onMouseMove(kwui::ButtonState button, int buttons, float x, float y) override;
    void onMouseUp(kwui::ButtonState button, int buttons, float x, float y) override;

private:
    struct Point
    {
        float x, y;
    };
    std::vector<std::unique_ptr<FormulaInterface>> formulas_;
    CartesianCoord coord_;
    std::optional<Point> mouse_down_pos_;
    // std::unique_ptr<ImplicitPlot> implicit_plot_;
};

kwui::CustomElement* PlotElementFactory();