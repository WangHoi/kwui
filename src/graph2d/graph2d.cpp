#include "graph2d.h"
#include "windows/graphics/GraphicDevice.h"
#include "windows/graphics/TextLayout.h"

namespace graph2d {

std::unique_ptr<TextLayoutInterface> createTextLayout(
    const std::string &text,
    const char* font_family,
    style::FontStyle font_style,
    style::FontWeight font_weight,
    float font_size)
{
    windows::graphics::TextLayoutBuilder b(text);
    b.SetFontFamily(font_family);
    b.FontStyle((windows::graphics::FontStyle)font_style);
    b.FontWeight(windows::graphics::FontWeight(font_weight.raw()));
    b.FontSize(font_size);
    return std::move(b.Build());
}

}