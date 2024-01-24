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

style::FontMetrics getFontMetrics(const char* font_family, float font_size)
{
    style::FontMetrics fm;
    DWRITE_FONT_METRICS dwrite_fm;
    auto GD = windows::graphics::GraphicDevice::instance();
    bool ok = GD->GetFontMetrics(font_family, dwrite_fm);
    if (!ok) {
        std::string default_font = GD->GetDefaultFontFamily();
        GD->GetFontMetrics(default_font, dwrite_fm);
    }
    const float factor = font_size / dwrite_fm.designUnitsPerEm;
    fm.ascent = dwrite_fm.ascent * factor;
    fm.descent = dwrite_fm.descent * factor;
    fm.line_gap = dwrite_fm.lineGap * factor;
    fm.cap_height = dwrite_fm.capHeight * factor;
    fm.x_height = dwrite_fm.xHeight * factor;
    return fm;
}

std::shared_ptr<BitmapInterface> createBitmap(const std::string& url)
{
    return std::shared_ptr<BitmapInterface>(new windows::graphics::BitmapImpl(url));
}

}