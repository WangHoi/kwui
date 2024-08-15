#pragma once
#include "api/kwui/CustomElementPaint.h"

namespace graph2d
{
using SweepDirection = kwui::SweepDirection;
using ArcSize = kwui::ArcSize;

/**
 * A complex, one-dimensional subset of a plane.
 * A path consists of a number of sub-paths, and a current point.
 * Sub-paths consist of segments of various types, such as lines, arcs, or beziers. Sub-paths can be open or closed, and can self-intersect.
 * Closed sub-paths enclose a (possibly discontiguous) region of the plane based on the current fillType.
 * The current point is initially at the origin. After each operation adding a segment to a sub-path, the current point is updated to the end of that segment.
 * Paths can be drawn on canvases using PaintContextInterface::drawPath, and can used to create clip regions using PaintContextInterface::clipPath.
 */
class PaintPathInterface
{
public:
    virtual ~PaintPathInterface() = default;
    virtual void moveTo(float x, float y) = 0;
    virtual void lineTo(float x, float y) = 0;
    virtual void arcTo(float radius_x, float radius_y, float rotation_degress,
                       SweepDirection sweep_dir, ArcSize arc_size, float x, float y) = 0;
    virtual void close() = 0;
};
}
