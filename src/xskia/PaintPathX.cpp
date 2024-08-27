#include "PaintPathX.h"

namespace xskia
{
void PaintPathX::moveTo(float x, float y)
{
    path_.moveTo(x, y);
}

void PaintPathX::lineTo(float x, float y)
{
    path_.lineTo(x, y);
}

void PaintPathX::arcTo(float radius_x, float radius_y, float rotation_degress, graph2d::SweepDirection sweep_dir,
                       graph2d::ArcSize arc_size, float x, float y)
{
    path_.arcTo(radius_x, radius_y, rotation_degress,
                (arc_size == graph2d::ArcSize::ARC_SIZE_SMALL) ? SkPath::kSmall_ArcSize : SkPath::kLarge_ArcSize,
                (sweep_dir == graph2d::SweepDirection::SWEEP_DIRECTION_CLOCKWISE)
                    ? SkPathDirection::kCW
                    : SkPathDirection::kCCW,
                x, y);
}

void PaintPathX::close()
{
    path_.close();
}
}
