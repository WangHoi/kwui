#include "TextLayoutD2D.h"
#include "GraphicDeviceD2D.h"
#include "base/EncodingManager.h"
#include <limits>

namespace windows {
namespace graphics {

TextLayoutD2D::TextLayoutD2D(const std::wstring& text, const std::string& font_family,
    float font_size, ComPtr<IDWriteTextLayout> layout)
    : _text(text)
    , _font_family(font_family)
    , _font_size(font_size)
    , _layout(layout) {
    UpdateTextMetrics();
}
void TextLayoutD2D::UpdateTextMetrics() {
    DWRITE_FONT_METRICS fm;
    if (GraphicDeviceD2D::instance()->getFontMetrics(_font_family, fm)) {
        _line_height = (fm.ascent + fm.descent + fm.lineGap) * _font_size / fm.designUnitsPerEm;
        _baseline = fm.ascent * _font_size / fm.designUnitsPerEm;
    } else {
        _line_height = _font_size;
        _baseline = _line_height * 0.8f;
    }
    DWRITE_TEXT_METRICS tm;
    if (SUCCEEDED(_layout->GetMetrics(&tm))) {
        //DWRITE_LINE_METRICS lm;
        //UINT lines;
        //_layout->GetLineMetrics(&lm, 1, &lines);
        _rect = scene2d::RectF::fromXYWH(tm.left, tm.top, tm.widthIncludingTrailingWhitespace, tm.height);
    } else {
        _rect = scene2d::RectF::fromZeros();
    }
}
scene2d::RectF TextLayoutD2D::caretRect(int idx) const {
    UINT index;
    BOOL trailing_hit;
    if (idx == _text.length()) {
        index = (idx > 0) ? (idx - 1) : 0;
        trailing_hit = TRUE;
    } else {
        index = idx;
        trailing_hit = FALSE;
    }
    DWRITE_HIT_TEST_METRICS htm;
    FLOAT x, y;
    HRESULT hr;
    hr = _layout->HitTestTextPosition(idx, trailing_hit, &x, &y, &htm);
    if (SUCCEEDED(hr)) {
        return MakeCaretRect(&htm);
    } else {
        return MakeCaretRect(nullptr);
    }
}
scene2d::RectF TextLayoutD2D::rangeRect(int start_idx, int end_idx) const {
    if (start_idx == end_idx)
        return scene2d::RectF::fromZeros();
    if (start_idx > end_idx)
        std::swap(start_idx, end_idx);
    scene2d::RectF r1 = caretRect(start_idx);
    scene2d::RectF r2 = caretRect(end_idx);
    return scene2d::RectF::fromLTRB(r1.left, r1.top, r2.right, r2.bottom);
}
int TextLayoutD2D::hitTest(const scene2d::PointF& pos, scene2d::RectF* out_caret_rect) const {
    BOOL trailing_hit = FALSE;
    BOOL inside = FALSE;
    DWRITE_HIT_TEST_METRICS htm;
    HRESULT hr;
    hr = _layout->HitTestPoint(pos.x, pos.y, &trailing_hit, &inside, &htm);
    if (SUCCEEDED(hr)) {
        if (out_caret_rect) *out_caret_rect = MakeCaretRect(&htm);
        return trailing_hit ? (htm.textPosition + 1) : htm.textPosition;
    } else {
        return -1;
    }
}
scene2d::RectF TextLayoutD2D::MakeCaretRect(const DWRITE_HIT_TEST_METRICS* htm) const {
    if (htm)
        return scene2d::RectF::fromXYWH(htm->left - 0.5f, 0, 1, _line_height);
    else
        return scene2d::RectF::fromXYWH(-0.5f, 0, 1, _line_height);
}

} // namespace graphics
} // namespace windows
