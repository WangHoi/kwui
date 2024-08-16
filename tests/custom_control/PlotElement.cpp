#include "PlotElement.h"

#include <algorithm>
#include <chrono>
#include <optional>

static std::chrono::time_point<std::chrono::system_clock> g_start_time_point = std::chrono::system_clock::now();

PlotElement::PlotElement()
{
    formulas_.push_back(SineWaveFormula::create());
    formulas_.push_back(SquareWaveFormula::create());
}

void PlotElement::onPaint(kwui::CustomElementPaintContextInterface& p, const kwui::CustomElementPaintOption& po)
{
    float sx = po.left;
    float sy = po.top;
    float ex = sx + po.width;
    float ey = sy + po.height;
    auto now = std::chrono::system_clock::now();
    float time = std::chrono::duration_cast<std::chrono::microseconds>(now - g_start_time_point).count();
    time /= 1000000.0f;

    coord_.setSceneBounds(sx, sy, ex, ey);
    coord_.setSceneScale(30.0f);

    for (float y = -5.0f; y <= 5.0f; y += 1.0f) {
        auto y1 = coord_.mapYToScene(y);
        auto path = kwui::CustomElementPaintPath::create();
        path->moveTo(sx, y1);
        path->lineTo(ex, y1);
        // p.drawPath(path.get());
    }

    for (const auto& formula : formulas_) {
        auto path = kwui::CustomElementPaintPath::create();
        bool first = true;
        for (float x = sx; x <= ex; x += 1.0f) {
            float x1 = coord_.mapXFromScene(x);
            float y1 = formula->evaluate(x1, time);
            float y = coord_.mapYToScene(y1);
            if (first) {
                first = false;
                path->moveTo(x, y);
            } else {
                if (sy - 1.0f < y && y < ey + 1.0f)
                    path->lineTo(x, y);
            }
        }
        p.drawPath(path.get());
    }
}

kwui::CustomElement* PlotElementFactory()
{
    return new PlotElement;
}
