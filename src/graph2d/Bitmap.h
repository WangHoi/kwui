#pragma once
#include "scene2d/geom_types.h"

namespace graph2d {

class BitmapInterface {
public:
    virtual ~BitmapInterface() = default;
    virtual const std::string& url() const = 0;
    virtual scene2d::DimensionF size() = 0;
};

}
