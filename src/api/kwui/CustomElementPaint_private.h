#pragma once

#include <memory>

namespace graph2d {
class PaintPathInterface;
}

namespace kwui {

class CustomElementPaintPath::Private
{
public:
    std::unique_ptr<graph2d::PaintPathInterface> path;
};


}
