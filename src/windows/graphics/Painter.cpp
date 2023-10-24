#include "Painter.h"
#include "base/log.h"

namespace windows {
namespace graphics {

Painter::Painter(ID2D1RenderTarget * rt, const scene2d::PointF& mouse_pos)
    : _rt(rt), _mouse_position(mouse_pos) {
    _rt->GetDpi(&_dpi_scale, &_dpi_scale);
    _dpi_scale /= USER_DEFAULT_SCREEN_DPI;
}

void Painter::Clear(const Color& c) {
    D2D1_COLOR_F color = D2D1::ColorF(c.GetRed(), c.GetGreen(), c.GetBlue(), c.GetAlpha());
    _rt->Clear(color);
}
void Painter::SetColor(const Color& c) {
    _current.color = c;
}
void Painter::SetStrokeColor(const Color& c) {
    _current.stroke_color = c;
}
void Painter::SetStrokeWidth(float w) {
    _current.stroke_width = w;
}
void Painter::DrawRect(float x, float y, float w, float h) {
    D2D1_RECT_F rect = D2D1::RectF(_current.offset.x + x, _current.offset.y + y,
                                   _current.offset.x + x + w, _current.offset.y + y + h);
    if (_current.pixel_snap)
        rect = PixelSnap(rect);

    if (_current.HasFill()) {
        ComPtr<ID2D1Brush> brush;
        if (_current.gradient_brush) {
            brush = _current.gradient_brush;
            brush->SetTransform(D2D1::Matrix3x2F::Translation(rect.left, rect.top));
        } else {
            brush = CreateBrush(_current.color);
        }
        _rt->FillRectangle(rect, brush.Get());
    }
    if (_current.HasStroke()) {
        ComPtr<ID2D1Brush> brush = CreateBrush(_current.stroke_color);
        _rt->DrawRectangle(rect, brush.Get(), _current.stroke_width);
    }
}
void Painter::DrawRoundedRect(float x, float y, float w, float h, float r) {
    D2D1_RECT_F rect = D2D1::RectF(_current.offset.x + x, _current.offset.y + y,
                                   _current.offset.x + x + w, _current.offset.y + y + h);
    if (_current.pixel_snap) {
        rect = PixelSnap(rect);
        r = PixelSnap(r);
    }

    D2D1_ROUNDED_RECT rrect = D2D1::RoundedRect(rect, r, r);
    if (_current.HasFill()) {
        ComPtr<ID2D1Brush> brush;
        if (_current.gradient_brush) {
            brush = _current.gradient_brush;
            brush->SetTransform(D2D1::Matrix3x2F::Translation(rect.left, rect.top));
        } else {
            brush = CreateBrush(_current.color);
        }
        _rt->FillRoundedRectangle(rrect, brush.Get());
    }
    if (_current.HasStroke()) {
        ComPtr<ID2D1Brush> brush = CreateBrush(_current.stroke_color);
        _rt->DrawRoundedRectangle(rrect, brush.Get(), _current.stroke_width);
    }
}
void Painter::DrawTextLayout(float x, float y, const TextLayout& layout) {
    D2D1_POINT_2F origin = D2D1::Point2F(_current.offset.x + x, _current.offset.y + y);
    if (_current.HasFill()) {
        ComPtr<ID2D1Brush> brush = CreateBrush(_current.color);
        if (_current.gradient_brush) {
            brush = _current.gradient_brush;
            brush->SetTransform(D2D1::Matrix3x2F::Translation(origin.x, origin.y));
        } else {
            brush = CreateBrush(_current.color);
        }
        _rt->DrawTextLayout(origin,
                            layout.GetRaw(),
                            brush.Get(),
                            D2D1_DRAW_TEXT_OPTIONS_NONE);
    }
}
void Painter::DrawGlyphRun(float x, float y, const GlyphRun& gr)
{
    D2D1_POINT_2F origin = D2D1::Point2F(_current.offset.x + x, _current.offset.y + y);
    if (_current.HasFill()) {
        ComPtr<ID2D1Brush> brush = CreateBrush(_current.color);
        if (_current.gradient_brush) {
            brush = _current.gradient_brush;
            brush->SetTransform(D2D1::Matrix3x2F::Translation(origin.x, origin.y));
        } else {
            brush = CreateBrush(_current.color);
        }
        _rt->DrawGlyphRun(origin,
            gr.raw(),
            brush.Get(),
            DWRITE_MEASURING_MODE_NATURAL);
    }
}
void Painter::Save() {
    _state_stack.push_back(_current);
}
void Painter::Restore() {
    if (!_state_stack.empty()) {
        _current = _state_stack.back();
        _state_stack.pop_back();
    } else {
        _current.Reset();
    }
}
ComPtr<ID2D1Brush> Painter::CreateBrush(const Color& c) {
    D2D1_COLOR_F color = D2D1::ColorF(c.GetRed(), c.GetGreen(), c.GetBlue(), c.GetAlpha());
    ComPtr<ID2D1SolidColorBrush> brush;
    HRESULT hr = _rt->CreateSolidColorBrush(color, brush.GetAddressOf());
    return brush;
}
D2D1_RECT_F Painter::PixelSnap(const D2D1_RECT_F& rect) {
    float width = PixelSnap(rect.right - rect.left);
    float height = PixelSnap(rect.bottom - rect.top);
    float left = PixelSnap(rect.left);
    float top = PixelSnap(rect.top);
    return D2D1::RectF(left, top, left + width, top + height);
}
D2D1_RECT_F Painter::PixelSnapConservative(const D2D1_RECT_F& rect) {
    float left = floorf(rect.left * _dpi_scale) / _dpi_scale;
    float top = floorf(rect.top * _dpi_scale) / _dpi_scale;
    float right = ceilf(rect.right * _dpi_scale) / _dpi_scale;
    float bottom = ceilf(rect.bottom * _dpi_scale) / _dpi_scale;
    return D2D1::RectF(left, top, right, bottom);
}
void Painter::Translate(const scene2d::PointF& offset) {
    _current.offset += offset;
    //LOG(INFO) << "Translate current: " << _current.offset;
}
void Painter::SetTranslation(const scene2d::PointF& abs_offset) {
    _current.offset = abs_offset;
    //LOG(INFO) << "SetTranslation current: " << _current.offset;
}
void Painter::PushClipRect(const scene2d::PointF& origin, const scene2d::DimensionF& size) {
    D2D1_RECT_F rect = D2D1::RectF(_current.offset.x + origin.x,
                                   _current.offset.y + origin.y,
                                   _current.offset.x + origin.x + size.width,
                                   _current.offset.y + origin.y + size.height);
    if (_current.pixel_snap)
        rect = PixelSnapConservative(rect);
    _rt->PushAxisAlignedClip(rect, D2D1_ANTIALIAS_MODE_ALIASED);
}
void Painter::PopClipRect() {
    _rt->PopAxisAlignedClip();
}
ComPtr<ID2D1Bitmap> Painter::CreateBitmap(const BitmapSubItem& item) {
    ComPtr<ID2D1Bitmap> bitmap;
    HRESULT hr;
    D2D1_BITMAP_PROPERTIES bitmap_props
        = D2D1::BitmapProperties(D2D1::PixelFormat(),
                                 item.dpi_scale.x * USER_DEFAULT_SCREEN_DPI,
                                 item.dpi_scale.y * USER_DEFAULT_SCREEN_DPI);
    hr = _rt->CreateBitmapFromWicBitmap(
        item.converter.Get(), bitmap_props, bitmap.GetAddressOf());
    if (FAILED(hr)) {
        LOG(ERROR) << "Painter::CreateBitmap failed, hr=" << std::hex << hr;
    }
    return bitmap;
}
void Painter::DrawBitmap(ID2D1Bitmap* bitmap, float x, float y, float w, float h) {
    D2D1_RECT_F rect = D2D1::RectF(_current.offset.x + x,
                                   _current.offset.y + y,
                                   _current.offset.x + x + w,
                                   _current.offset.y + y + h);
    if (_current.pixel_snap)
        rect = PixelSnap(rect);
    _rt->DrawBitmap(bitmap, rect, 1.0);
}
void Painter::DrawScale9Bitmap(ID2D1Bitmap* bitmap, float x, float y, float w, float h,
                               float src_margin, float dst_margin) {
    D2D1_SIZE_F bitmap_size = bitmap->GetSize();
    D2D1_RECT_F src_rect, dst_rect;
    // TopLeft 
    src_rect = D2D1::RectF(0,
                           0,
                           src_margin,
                           src_margin);
    dst_rect = D2D1::RectF(_current.offset.x + x,
                           _current.offset.y + y,
                           _current.offset.x + x + dst_margin,
                           _current.offset.y + y + dst_margin);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);
    // Top
    src_rect = D2D1::RectF(src_margin,
                           0,
                           bitmap_size.width - src_margin,
                           src_margin);
    dst_rect = D2D1::RectF(_current.offset.x + x + dst_margin,
                           _current.offset.y + y,
                           _current.offset.x + x + w - dst_margin,
                           _current.offset.y + y + dst_margin);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);
    // TopRight
    src_rect = D2D1::RectF(bitmap_size.width - src_margin,
                           0,
                           bitmap_size.width,
                           src_margin);
    dst_rect = D2D1::RectF(_current.offset.x + x + w - dst_margin,
                           _current.offset.y + y,
                           _current.offset.x + x + w,
                           _current.offset.y + y + dst_margin);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);


    // Left
    src_rect = D2D1::RectF(0,
                           src_margin,
                           src_margin,
                           bitmap_size.height - src_margin);
    dst_rect = D2D1::RectF(_current.offset.x + x,
                           _current.offset.y + y + dst_margin,
                           _current.offset.x + x + dst_margin,
                           _current.offset.y + y + h - dst_margin);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);
    // Center
    src_rect = D2D1::RectF(src_margin,
                           src_margin,
                           bitmap_size.width - src_margin,
                           bitmap_size.height - src_margin);
    dst_rect = D2D1::RectF(_current.offset.x + x + dst_margin,
                           _current.offset.y + y + dst_margin,
                           _current.offset.x + x + w - dst_margin,
                           _current.offset.y + y + h - dst_margin);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);
    // Right
    src_rect = D2D1::RectF(bitmap_size.width - src_margin,
                           src_margin,
                           bitmap_size.width,
                           bitmap_size.height - src_margin);
    dst_rect = D2D1::RectF(_current.offset.x + x + w - dst_margin,
                           _current.offset.y + y + dst_margin,
                           _current.offset.x + x + w,
                           _current.offset.y + y + h - dst_margin);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);

    // BottomLeft
    src_rect = D2D1::RectF(0,
                           bitmap_size.height - src_margin,
                           src_margin,
                           bitmap_size.height);
    dst_rect = D2D1::RectF(_current.offset.x + x,
                           _current.offset.y + y + h - dst_margin,
                           _current.offset.x + x + dst_margin,
                           _current.offset.y + y + h);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);
    // Bottom
    src_rect = D2D1::RectF(src_margin,
                           bitmap_size.height - src_margin,
                           bitmap_size.width - src_margin,
                           bitmap_size.height);
    dst_rect = D2D1::RectF(_current.offset.x + x + dst_margin,
                           _current.offset.y + y + h - dst_margin,
                           _current.offset.x + x + w - dst_margin,
                           _current.offset.y + y + h);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);
    // BottomRight
    src_rect = D2D1::RectF(bitmap_size.width - src_margin,
                           bitmap_size.height - src_margin,
                           bitmap_size.width,
                           bitmap_size.height);
    dst_rect = D2D1::RectF(_current.offset.x + x + w - dst_margin,
                           _current.offset.y + y + h - dst_margin,
                           _current.offset.x + x + w,
                           _current.offset.y + y + h);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);
}

ComPtr<ID2D1LinearGradientBrush> Painter::CreateLinearGradientBrush_Logo() {
    HRESULT hr;
    ComPtr<ID2D1LinearGradientBrush> brush;
    ComPtr<ID2D1GradientStopCollection> pGradientStops;
    Color c0 = Color::FromString(/*"#FF8833"*/"#FF6A00");
    Color c1 = Color::FromString("#FF3F00");
    //c0 = RED;
    //c1 = BLUE;
    D2D1_GRADIENT_STOP gradientStops[2];
    gradientStops[0].color = D2D1::ColorF(c0.GetRed(), c0.GetGreen(), c0.GetBlue(), c0.GetAlpha());
    gradientStops[0].position = 0.0f;
    gradientStops[1].color = D2D1::ColorF(c1.GetRed(), c1.GetGreen(), c1.GetBlue(), c1.GetAlpha());
    gradientStops[1].position = 1.0f;
    hr = _rt->CreateGradientStopCollection(
        gradientStops,
        2,
        D2D1_GAMMA_1_0,
        D2D1_EXTEND_MODE_MIRROR,
        pGradientStops.GetAddressOf()
    );

    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES props
        = D2D1::LinearGradientBrushProperties(D2D1::Point2F(40, 2), D2D1::Point2F(40, 80));
    hr = _rt->CreateLinearGradientBrush(props, pGradientStops.Get(), brush.GetAddressOf());
    if (FAILED(hr)) {
        LOG(ERROR) << "Create LinearGradientBrush failed, hr=" << std::hex << hr;
    }
    return brush;
}
ComPtr<ID2D1RadialGradientBrush> Painter::CreateRadialGradientBrush_Highlight() {
    HRESULT hr;
    ComPtr<ID2D1RadialGradientBrush> brush;
    ComPtr<ID2D1GradientStopCollection> pGradientStops;
    Color c0 = Color::FromString("#FFFFFF40");
    Color c1 = Color::FromString("#FFFFFF00");
    D2D1_GRADIENT_STOP gradientStops[2];
    gradientStops[0].color = D2D1::ColorF(c0.GetRed(), c0.GetGreen(), c0.GetBlue(), c0.GetAlpha());
    gradientStops[0].position = 0.0f;
    gradientStops[1].color = D2D1::ColorF(c1.GetRed(), c1.GetGreen(), c1.GetBlue(), c1.GetAlpha());
    gradientStops[1].position = 1.0f;
    hr = _rt->CreateGradientStopCollection(
        gradientStops,
        2,
        D2D1_GAMMA_1_0,
        D2D1_EXTEND_MODE_MIRROR,
        pGradientStops.GetAddressOf()
    );

    D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES props
        = D2D1::RadialGradientBrushProperties(D2D1::Point2F(32, 32), D2D1::Point2F(), 32, 32);
    hr = _rt->CreateRadialGradientBrush(props, pGradientStops.Get(), brush.GetAddressOf());
    if (FAILED(hr)) {
        LOG(ERROR) << "Create RadialGradientBrush failed, hr=" << std::hex << hr;
    }
    return brush;
}
void Painter::SetBrush(ComPtr<ID2D1Brush> brush) {
    _current.gradient_brush = brush;
}

} // namespace graphics
} // namespace windows
