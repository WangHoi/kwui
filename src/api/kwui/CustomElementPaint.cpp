#include "CustomElementPaint.h"
#include "CustomElementPaint_private.h"
#include "graph2d/PaintPathInterface.h"
#include <memory>

#include "graph2d/graph2d.h"


namespace kwui
{
void CustomElementPaintPath::moveTo(float x, float y)
{
    d->path->moveTo(x, y);
}

void CustomElementPaintPath::lineTo(float x, float y)
{
    d->path->lineTo(x, y);
}

void CustomElementPaintPath::arcTo(float radius_x, float radius_y, float rotation_degress, SweepDirection sweep_dir,
                                   ArcSize arc_size, float x, float y)
{
    d->path->arcTo(radius_x, radius_y, rotation_degress, sweep_dir, arc_size, x, y);
}

void CustomElementPaintPath::close()
{
    d->path->close();
}

std::unique_ptr<CustomElementPaintPath> CustomElementPaintPath::create()
{
    return std::unique_ptr<CustomElementPaintPath>(new CustomElementPaintPath);
}

CustomElementPaintPath::CustomElementPaintPath()
    : d(new Private)
{
    d->path = graph2d::createPath();
}

CustomElementPaintBrush::~CustomElementPaintBrush()
{
    delete d;
}

void CustomElementPaintBrush::setStroke(bool stroke)
{
    const auto style = stroke ? graph2d::PAINT_STYLE_STROKE : graph2d::PAINT_STYLE_FILL; 
    d->brush.setStyle(style);
}

void CustomElementPaintBrush::setColor(int red, int green, int blue, int alpha)
{
    const auto color = style::Color(red, green, blue, alpha);
    d->brush.setColor(color);
}

void CustomElementPaintBrush::setStrokeWidth(float stroke_width)
{
    d->brush.setStrokeWidth(stroke_width);
}

std::unique_ptr<CustomElementPaintBrush> CustomElementPaintBrush::create()
{
    return std::unique_ptr<CustomElementPaintBrush>(new CustomElementPaintBrush);
}

CustomElementPaintBrush::CustomElementPaintBrush()
    : d(new Private)
{
    d->brush.setStrokeJoin(graph2d::STROKE_JOIN_BEVEL);
}

CustomElementPaintFont::~CustomElementPaintFont()
{
    delete d;
}

std::unique_ptr<CustomElementPaintFont> CustomElementPaintFont::create(const std::string& font_name, float point_size)
{
    return std::unique_ptr<CustomElementPaintFont>(new CustomElementPaintFont(font_name, point_size));
}

CustomElementPaintFont::CustomElementPaintFont(const std::string& font_name, float point_size)
    : d(new Private)
{
    d->font_name = font_name;
    d->point_size = point_size;
}

CustomElementPaintPath::~CustomElementPaintPath()
{
    delete d;
}

}
