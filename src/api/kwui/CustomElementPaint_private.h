#pragma once

#include <memory>

#include "graph2d/PaintBrush.h"

namespace graph2d {
class PaintPathInterface;
}

namespace kwui {

class CustomElementPaintPath::Private
{
public:
    std::unique_ptr<graph2d::PaintPathInterface> path;
};

class CustomElementPaintBrush::Private
{
public:
    graph2d::PaintBrush brush;
};

class CustomElementPaintFont::Private
{
public:
    std::string font_name;
    float point_size = 14.0f;
};

}
