#pragma once

#include "kwui/CustomElement.h"
#include <vector>

#include "Formula.h"
#include "CartesianCoord.h"

class PlotElement : public kwui::CustomElement
{
public:
    PlotElement();
    void onPaint(kwui::CustomElementPaintContextInterface& p, const kwui::CustomElementPaintOption& po) override;

private:
    std::vector<std::unique_ptr<FormulaInterface>> formulas_;
    CartesianCoord coord_;
};

kwui::CustomElement* PlotElementFactory();