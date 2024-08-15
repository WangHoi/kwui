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

CustomElementPaintPath CustomElementPaintPath::create()
{
    return CustomElementPaintPath();
}

CustomElementPaintPath::CustomElementPaintPath()
    : d(new Private)
{
    d->path = graph2d::createPath();
}

CustomElementPaintPath::~CustomElementPaintPath()
{
    delete d;
}
}
