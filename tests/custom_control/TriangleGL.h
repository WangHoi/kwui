#pragma once
#include "windows/windows_header.h"
#include <kwui/CustomElement.h>

class TriangleGL
{
public:
    static void draw(ID3D11Device1* device,
                     kwui::CustomElementPaintContextInterface& painter,
                     const kwui::CustomElementPaintOption& po);
};
