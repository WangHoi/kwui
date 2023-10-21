#include "graph2d.h"
#include "windows/graphics/GraphicDevice.h"
#include "windows/graphics/TextLayout.h"
#include "windows/graphics/TextFlow.h"
#include "windows/EncodingManager.h"

namespace graph2d {

std::unique_ptr<TextFlowInterface> createTextFlow(
    const std::string& text,
    const char* font_family,
    style::FontStyle font_style,
    style::FontWeight font_weight,
    float font_size)
{
    std::wstring u16_text = windows::EncodingManager::UTF8ToWide(text);
    windows::graphics::FontWeight win_font_weight(font_weight.raw());
    windows::graphics::FontStyle win_font_style = (windows::graphics::FontStyle)font_style;
    return windows::graphics::GraphicDevice::instance()
        ->CreateTextFlow(u16_text, font_family, font_size, win_font_weight, win_font_style);
}

}