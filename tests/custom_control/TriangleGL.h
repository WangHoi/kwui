#pragma once
#include <kwui/CustomElement.h>

class TriangleGL
{
public:
    static void draw(void*,
                     kwui::CustomElementPaintContextInterface& painter,
                     const kwui::CustomElementPaintOption& po);
};
