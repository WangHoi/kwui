#pragma once

#include "kwui_export.h"
#include <memory>
#include <string>

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

enum KWUI_EXPORT ColorType
{
    COLOR_TYPE_ALPHA8,
    COLOR_TYPE_BGRA8888,
    COLOR_TYPE_RGBA8888,
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

class KWUI_EXPORT CustomElementPaintBrush
{
public:
    ~CustomElementPaintBrush();

    void setStroke(bool stroke = true);
    void setColor(int red, int green, int blue, int alpha = 0xff);
    void setColor(uint32_t rgb, int alpha = 0xff);
    void setStrokeWidth(float stroke_width);

    static std::unique_ptr<CustomElementPaintBrush> create();

private:
    CustomElementPaintBrush();
    CustomElementPaintBrush(const CustomElementPaintBrush&) = delete;
    CustomElementPaintBrush& operator =(const CustomElementPaintBrush&) = delete;

    class Private;
    Private* d;
    friend class ::scene2d::control::CustomElementControl;
};

class KWUI_EXPORT CustomElementPaintFont
{
public:
    ~CustomElementPaintFont();

    static std::unique_ptr<CustomElementPaintFont> create(const std::string& font_name, float point_size);

private:
    CustomElementPaintFont(const std::string& font_name, float point_size);
    CustomElementPaintFont(const CustomElementPaintFont&) = delete;
    CustomElementPaintFont& operator =(const CustomElementPaintFont&) = delete;

    class Private;
    Private* d;
    friend class ::scene2d::control::CustomElementControl;
};

}
