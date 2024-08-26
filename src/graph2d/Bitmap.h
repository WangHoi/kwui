#pragma once
#include "scene2d/geom_types.h"

namespace graph2d {

class BitmapInterface {
public:
    virtual ~BitmapInterface() = default;
    virtual scene2d::DimensionF size() const = 0;
};

}
