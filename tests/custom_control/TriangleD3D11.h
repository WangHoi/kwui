#pragma once
#include "windows/windows_header.h"
#include <kwui/CustomElement.h>

class TriangleD3D11
{
public:
    static void draw(ID3D11Device1* device,
                     kwui::CustomElementPaintContextInterface& painter,
                     const kwui::CustomElementPaintOption& po);
};
