#include "graph2d.h"
#include "windows/graphics/GraphicDevice.h"
#include "windows/graphics/TextLayout.h"
#include "windows/graphics/TextFlow.h"
#include "windows/graphics/Painter.h"
#include "windows/EncodingManager.h"

namespace graph2d {

std::unique_ptr<TextFlowInterface> createTextFlow(
    const std::string& text,
    float line_height,
    const char* font_family,
    style::FontStyle font_style,
    style::FontWeight font_weight,
    float font_size)
{
    std::wstring u16_text = windows::EncodingManager::UTF8ToWide(text);
    windows::graphics::FontWeight win_font_weight(font_weight.raw());
    windows::graphics::FontStyle win_font_style = (windows::graphics::FontStyle)font_style;
    return windows::graphics::GraphicDevice::instance()
        ->CreateTextFlow(u16_text, line_height, font_family, font_size, win_font_weight, win_font_style);
}

FlowMetrics getFontMetrics(const char* font_family, float font_size)
{
    FlowMetrics fm;
    DWRITE_FONT_METRICS dwrite_fm;
    if (windows::graphics::GraphicDevice::instance()->GetFontMetrics(font_family, dwrite_fm)) {
        fm.line_height = (dwrite_fm.ascent + dwrite_fm.descent + dwrite_fm.lineGap) * font_size / dwrite_fm.designUnitsPerEm;
        fm.baseline = dwrite_fm.ascent * font_size / dwrite_fm.designUnitsPerEm;
    } else {
        LOG(ERROR) << "graph2d::getFontMetrics failed, \"" << font_family << "\" not found.";
        fm.line_height = font_size;
        fm.baseline = fm.line_height * 0.8f;
    }

    return fm;
}

std::shared_ptr<BitmapInterface> createBitmap(const std::string& url)
{
    return std::shared_ptr<BitmapInterface>(new windows::graphics::BitmapImpl(url));
}

}