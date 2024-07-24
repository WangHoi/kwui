#include "TextLayout.h"
#include "base/EncodingManager.h"
#if WITH_SKIA
#include "xskia/TextLayoutX.h"
#endif
#ifdef _WIN32
#include "windows/graphics/GraphicDeviceD2D.h"
#include "windows/graphics/TextLayoutD2D.h"
#endif
#include <limits>

namespace scene2d {

const float DEFAULT_FONT_SIZE = 14;
const char* DEFAULT_FONT_FAMILY = "Microsoft YaHei";

TextLayoutBuilder::TextLayoutBuilder(const std::string& text) {
    std::wstring utf16_text = base::EncodingManager::UTF8ToWide(text);
    Init(utf16_text);
}
TextLayoutBuilder::TextLayoutBuilder(const std::wstring& text) {
    Init(text);
}
void TextLayoutBuilder::Init(const std::wstring& text) {
    _text = text;
    _max_width = std::numeric_limits<float>::max();
    _font_family = DEFAULT_FONT_FAMILY;
    _font_size = DEFAULT_FONT_SIZE;
    _font_style = style::FontStyle::Normal;
    _align = TEXT_ALIGN_TOP_LEFT;
}

std::unique_ptr<TextLayoutInterface> TextLayoutBuilder::Build() {
#if WITH_SKIA
    return std::make_unique<::xskia::TextLayoutX>(_text, _font_family, _font_size);
#elif defined(_WIN32)
    ComPtr<IDWriteTextLayout> layout = windows::graphics::GraphicDeviceD2D::instance()
        ->createTextLayout(_text, _font_family, _font_size, _font_weight, _font_style);
    layout->SetMaxWidth(_max_width);
    DWRITE_TEXT_ALIGNMENT align = DWRITE_TEXT_ALIGNMENT_LEADING;
    if (_align & TEXT_ALIGN_LEFT)
        align = DWRITE_TEXT_ALIGNMENT_LEADING;
    else if (_align & TEXT_ALIGN_CENTER)
        align = DWRITE_TEXT_ALIGNMENT_CENTER;
    else if (_align & TEXT_ALIGN_RIGHT)
        align = DWRITE_TEXT_ALIGNMENT_TRAILING;
    layout->SetTextAlignment(align);
    return std::make_unique<::windows::graphics::TextLayoutD2D>(_text, _font_family, _font_size, layout);
#else
    LOG(ERROR) << "TextLayoutBuilder::Build() not implemented.";
    return nullptr;
#endif
}
} // namespace scene2d
