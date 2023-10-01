#include "graph2d.h"
#include "windows/graphics/GraphicDevice.h"
#include "windows/graphics/TextLayout.h"

namespace graph2d {

std::unique_ptr<TextLayoutInterface> createTextLayout(const std::string &text)
{
    windows::graphics::TextLayoutBuilder b(text);
    return std::move(b.Build());
}

}