#include "graph2d.h"
#ifdef _WIN32
#include "windows/graphics/GraphicDevice.h"
#include "windows/graphics/TextLayout.h"
#include "windows/graphics/TextFlow.h"
#include "windows/graphics/Painter.h"
#include "windows/EncodingManager.h"
#endif

namespace graph2d {

std::unique_ptr<TextFlowInterface> createTextFlow(
    const std::string& text,
    float line_height,
    const char* font_family,
    style::FontStyle font_style,
    style::FontWeight font_weight,
    float font_size)
{
#ifdef _WIN32
    std::wstring u16_text = windows::EncodingManager::UTF8ToWide(text);
    windows::graphics::FontWeight win_font_weight(font_weight.raw());
    windows::graphics::FontStyle win_font_style = (windows::graphics::FontStyle)font_style;
    return windows::graphics::GraphicDevice::instance()
        ->createTextFlow(u16_text, line_height, font_family, font_size, win_font_weight, win_font_style);
#else
#pragma message("TODO: implement graph2d::createTextFlow().")
    return nullptr;
#endif
}

void updateTextFlow(std::unique_ptr<TextFlowInterface>& text_flow, const std::string& text, float line_height, const char* font_family, style::FontStyle font_style, style::FontWeight font_weight, float font_size)
{
#ifdef _WIN32
    std::wstring u16_text = windows::EncodingManager::UTF8ToWide(text);
    windows::graphics::FontWeight win_font_weight(font_weight.raw());
    windows::graphics::FontStyle win_font_style = (windows::graphics::FontStyle)font_style;
    windows::graphics::TextFlow* win_tf = static_cast<windows::graphics::TextFlow*>(text_flow.get());
    return windows::graphics::GraphicDevice::instance()
        ->updateTextFlow(win_tf, u16_text, line_height, font_family, font_size, win_font_weight, win_font_style);
#else
#pragma message("TODO: implement graph2d::updateTextFlow().")
#endif
}

style::FontMetrics getFontMetrics(const char* font_family, float font_size)
{
#ifdef _WIN32
    style::FontMetrics fm;
    DWRITE_FONT_METRICS dwrite_fm;
    auto GD = windows::graphics::GraphicDevice::instance();
    bool ok = GD->getFontMetrics(font_family, dwrite_fm);
    if (!ok) {
        std::string default_font = GD->getDefaultFontFamily();
        GD->getFontMetrics(default_font, dwrite_fm);
    }
    const float factor = font_size / dwrite_fm.designUnitsPerEm;
    fm.ascent = dwrite_fm.ascent * factor;
    fm.descent = dwrite_fm.descent * factor;
    fm.line_gap = dwrite_fm.lineGap * factor;
    fm.cap_height = dwrite_fm.capHeight * factor;
    fm.x_height = dwrite_fm.xHeight * factor;
    fm.underline_offset = dwrite_fm.underlinePosition * factor;
    fm.underline_thickness = dwrite_fm.underlineThickness * factor;
    fm.line_through_offset = dwrite_fm.strikethroughPosition * factor;
    fm.line_through_thickness = dwrite_fm.strikethroughThickness * factor;
    return fm;
#else
#pragma message("TODO: implement graph2d::createTextFlow().")
    return style::FontMetrics();
#endif
}

std::shared_ptr<BitmapInterface> createBitmap(const std::string& url)
{
#ifdef _WIN32
    return std::shared_ptr<BitmapInterface>(new windows::graphics::BitmapImpl(url));
#else
#pragma message("TODO: implement graph2d::createTextFlow().")
    return nullptr;
#endif
}

}