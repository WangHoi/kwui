#pragma once
#include <include/core/SkPath.h>

#include "graph2d/PaintPathInterface.h"

namespace xskia
{
class PaintPathX : public graph2d::PaintPathInterface
{
public:
    void moveTo(float x, float y) override;
    void lineTo(float x, float y) override;
    void arcTo(float radius_x, float radius_y, float rotation_degress, graph2d::SweepDirection sweep_dir,
        graph2d::ArcSize arc_size, float x, float y) override;
    void close() override;

    const SkPath& skPath() const
    {
        return path_;
    }

private:
    SkPath path_;
};
}
