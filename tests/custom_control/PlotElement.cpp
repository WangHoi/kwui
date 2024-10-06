#include "PlotElement.h"
#ifdef _WIN32
#include "TriangleD3D11.h"
#endif
#include <algorithm>
#include <chrono>
#include <optional>

static std::chrono::time_point<std::chrono::system_clock> g_start_time_point = std::chrono::system_clock::now();

static const uint32_t BACKGROUND_COLOR = 0x202020;
static const uint32_t GRID_COLOR = 0x606060;
static const uint32_t GRID_THIN_COLOR = 0x404040;
static const uint32_t FORMULAR_COLORS[] = {0xffc040, 0xffffa0, 0xa0ffc0, 0x40c0ff, 0xd0a0ff, 0xff80b0};

PlotElement::PlotElement()
{
    formulas_.push_back(SquareWaveFormula::create());
    formulas_.push_back(SineWaveFormula::create());
    formulas_.push_back(MultiSineWaveFormula::create());
    // implicit_plot_ = std::make_unique<ImplicitPlot>();
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

    // Update implicit function
    // auto [tl_x, tl_y] = coord_.mapFromScene(sx, sy);
    // auto [br_x, br_y] = coord_.mapFromScene(ex, ey);
    // implicit_plot_->update(p.getDpiScale(), roundf(po.width), roundf(po.height), {tl_x, br_x}, {br_y, tl_y});

    // Draw axis
    {
        drawAxis(p, -1, sx, sy, ex, ey);
        drawAxis(p, 0, sx, sy, ex, ey);
    }

    // Draw formulas
    {
        auto brush = kwui::CustomElementPaintBrush::create();
        brush->setStroke();
        brush->setStrokeWidth(2);
        size_t i = 0;
        float xstep = 1.0f / p.getDpiScale();
        for (const auto& formula : formulas_) {
            auto path = kwui::CustomElementPaintPath::create();
            bool first = true;
            for (float x = sx; x <= ex; x += xstep) {
                float y1 = formula->evaluate(coord_.mapXFromScene(x), time);
                float y = coord_.mapYToScene(y1);
                if (first) {
                    first = false;
                    path->moveTo(x, y);
                } else {
                    path->lineTo(x, y);
                }
            }
            brush->setColor(FORMULAR_COLORS[i++]);
            p.drawPath(*path, *brush);
        }
    }

    // Draw implicit function
    // p.writePixels(implicit_plot_->imageData(), roundf(po.width), roundf(po.height), roundf(po.width) * 4,
    //               sx, sy, kwui::COLOR_TYPE_RGBA8888);

    // Draw native
    if (auto idata = kwui::Application::instance()->internalData()) {
#ifdef _WIN32
        if (idata->renderer_type == kwui::INTERNAL_RENDERER_D3D11_1) {
            TriangleD3D11::draw(static_cast<ID3D11Device1*>(idata->context), p, po);
        }
#endif
    }
}

void PlotElement::drawAxis(kwui::CustomElementPaintContextInterface& p, float offset, float sx, float sy, float ex,
                           float ey)
{
    auto scale_factor = coord_.getScaleFactor();
    auto font = kwui::CustomElementPaintFont::create("", 9);

    auto brush = kwui::CustomElementPaintBrush::create();
    brush->setStroke();
    brush->setStrokeWidth(2 + offset);

    brush->setColor(offset == 0 ? GRID_COLOR : GRID_THIN_COLOR);

    auto text_brush = kwui::CustomElementPaintBrush::create();
    text_brush->setColor(GRID_COLOR);

    auto path = kwui::CustomElementPaintPath::create();

    static constexpr float SEP = 5;
    auto [left, top] = coord_.mapFromScene(sx, sy);
    auto [right, bottom] = coord_.mapFromScene(ex, ey);
    auto min_interval = std::min(right - left, top - bottom);
    float step = powf(SEP, offset + floorf(logf(min_interval) / logf(SEP)));
    float ileft = floorf(left / step) * step;
    float ibottom = floorf(bottom / step) * step;
    bool is_istep = fabsf(step - roundf(step)) < 1e-3f;

    std::vector<std::tuple<float, float, std::string>> x_axis_labels, y_axis_labels;

    // Vertical axis
    for (float x = ileft; x <= right; x += step) {
        auto x1 = coord_.mapXToScene(x);
        if (sx <= x1 && x1 <= ex) {
            path->moveTo(x1, sy);
            path->lineTo(x1, ey);
        }
        if (offset == 0.0f) {
            char buf[32] = {};
            if (is_istep) {
                snprintf(buf, sizeof(buf) - 1, "%.0f", x);
            } else {
                snprintf(buf, sizeof(buf) - 1, "%.02f", x);
            }
            x_axis_labels.push_back(std::tuple<float, float, std::string>(x1 + 2, ey - 18, buf));
        }
    }


    // Horizontal axis
    for (float y = ibottom; y <= top; y += step) {
        auto y1 = coord_.mapYToScene(y);
        if (sy <= y1 && y1 <= ey) {
            path->moveTo(sx, y1);
            path->lineTo(ex, y1);
        }

        if (offset == 0.0f) {
            char buf[32] = {};
            if (is_istep) {
                snprintf(buf, sizeof(buf) - 1, "%.0f", y);
            } else {
                snprintf(buf, sizeof(buf) - 1, "%.02f", y);
            }
            y_axis_labels.push_back(std::tuple<float, float, std::string>(sx + 2, y1, buf));
        }
    }

    p.drawPath(*path, *brush);

    // Draw labels
    for (auto& [x, y, text] : x_axis_labels) {
        p.drawText(text, x, y, *text_brush, *font);
    }
    for (auto& [x, y, text] : y_axis_labels) {
        p.drawText(text, x, y, *text_brush, *font);
    }
}

void PlotElement::onWheel(float wheel_delta)
{
    if (wheel_delta > 0)
        coord_.zoomIn();
    else
        coord_.zoomOut();
}

void PlotElement::onMouseDown(kwui::ButtonState button, int buttons, float x, float y)
{
    mouse_down_pos_ = Point{x, y};
}

void PlotElement::onMouseMove(kwui::ButtonState button, int buttons, float x, float y)
{
    if (mouse_down_pos_.has_value()) {
        auto [ox, oy] = mouse_down_pos_.value();
        coord_.pan(x - ox, y - oy);
        mouse_down_pos_ = Point{x, y};
    }
}

void PlotElement::onMouseUp(kwui::ButtonState button, int buttons, float x, float y)
{
    mouse_down_pos_ = std::nullopt;
}

kwui::CustomElement* PlotElementFactory()
{
    return new PlotElement;
}
