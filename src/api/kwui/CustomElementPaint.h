#pragma once

#include "kwui_export.h"
#include <memory>

namespace scene2d::control
{
class CustomElementControl;
}

namespace kwui
{
enum KWUI_EXPORT SweepDirection
{
    SWEEP_DIRECTION_CLOCKWISE,
    SWEEP_DIRECTION_COUNTER_CLOCKWISE,
};

enum KWUI_EXPORT ArcSize
{
    ARC_SIZE_SMALL,
    ARC_SIZE_LARGE,
};

class KWUI_EXPORT CustomElementPaintPath
{
public:
    ~CustomElementPaintPath();
    void moveTo(float x, float y);
    void lineTo(float x, float y);
    void arcTo(float radius_x, float radius_y, float rotation_degress,
               SweepDirection sweep_dir, ArcSize arc_size, float x, float y);
    void close();

    static std::unique_ptr<CustomElementPaintPath> create();

private:
    CustomElementPaintPath();
    CustomElementPaintPath(const CustomElementPaintPath&) = delete;
    CustomElementPaintPath& operator =(const CustomElementPaintPath&) = delete;

    class Private;
    Private* d;

    friend class ::scene2d::control::CustomElementControl;
};

}
