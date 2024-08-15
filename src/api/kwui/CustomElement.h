#pragma once
#include "kwui_export.h"

#include "CustomElementPaint.h"

namespace kwui
{
class CustomElement;
class CustomElementPaintContextInterface;
struct CustomElementPaintOption;
class CustomElementPaintPath;

typedef CustomElement* (*CustomElementFactoryFn)();

class KWUI_EXPORT CustomElement
{
public:
    virtual ~CustomElement() = default;
    virtual void onPaint(CustomElementPaintContextInterface& p, const CustomElementPaintOption& po) = 0;
};

struct KWUI_EXPORT CustomElementPaintOption
{
    float left = 0;
    float top = 0;
    float width = 0;
    float height = 0;
};

class KWUI_EXPORT CustomElementPaintContextInterface
{
public:
    virtual ~CustomElementPaintContextInterface() = default;
    virtual void* getNativeBitmap(float& out_pixel_width, float& out_pixel_height) = 0;
    virtual void setFillBitmap(void* native_bitmap) = 0;
    virtual void drawRoundedRect(float left, float top, float width, float height, float radius) = 0;
    virtual void drawPath(const CustomElementPaintPath* path) = 0;
};

}
