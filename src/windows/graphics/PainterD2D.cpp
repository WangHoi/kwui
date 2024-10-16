#include "PainterD2D.h"
#include "base/log.h"
#define _USE_MATH_DEFINES 1
#include <math.h>

#include "PaintSurfaceD2D.h"
#include "graph2d/image_blur.h"

namespace windows::graphics
{
static constexpr float PADDING_PIXELS = 4;

Painter::Painter(ID2D1RenderTarget* rt, const scene2d::PointF& mouse_pos)
    : _rt(rt), _mouse_position(mouse_pos)
{
    _rt->GetDpi(&_dpi_scale, &_dpi_scale);
    _dpi_scale /= USER_DEFAULT_SCREEN_DPI;
}

void Painter::Clear(const style::Color& c)
{
    D2D1_COLOR_F color = D2D1::ColorF(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
    _rt->Clear(color);
}

void Painter::SetColor(const style::Color& c)
{
    _current.color = c;
}

void Painter::SetStrokeColor(const style::Color& c)
{
    _current.stroke_color = c;
}

void Painter::SetStrokeWidth(float w)
{
    _current.stroke_width = w;
}

void Painter::DrawRect(float x, float y, float w, float h)
{
    D2D1_RECT_F stroke_rect = D2D1::RectF(x, y,
                                          x + w, y + h);
    if (_current.pixel_snap)
        stroke_rect = PixelSnap(stroke_rect);

    D2D1_RECT_F fill_rect = stroke_rect;
    if (_current.stroke_width > 0)
    {
        const float stroke_width = std::max(0.0f,
                                            std::min({
                                                0.5f * (stroke_rect.right - stroke_rect.left),
                                                0.5f * (stroke_rect.bottom - stroke_rect.top),
                                                _current.stroke_width
                                            }));
        fill_rect.left += stroke_width;
        fill_rect.top += stroke_width;
        fill_rect.right -= stroke_width;
        fill_rect.bottom -= stroke_width;

        const float half_width = 0.5f * stroke_width;
        stroke_rect.left += half_width;
        stroke_rect.top += half_width;
        stroke_rect.right -= half_width;
        stroke_rect.bottom -= half_width;
    }

    if (_current.HasFill())
    {
        ComPtr<ID2D1Brush> brush;
        if (_current.gradient_brush)
        {
            brush = _current.gradient_brush;
            brush->SetTransform(D2D1::Matrix3x2F::Translation(fill_rect.left, fill_rect.top));
        }
        else
        {
            brush = CreateBrush(_current.color);
        }
        _rt->FillRectangle(fill_rect, brush.Get());
    }
    if (_current.HasStroke())
    {
        ComPtr<ID2D1Brush> brush = CreateBrush(_current.stroke_color);
        _rt->DrawRectangle(stroke_rect, brush.Get(), _current.stroke_width);
    }
}

void Painter::DrawRoundedRect(float x, float y, float w, float h, float r)
{
    D2D1_RECT_F rect = D2D1::RectF(x, y,
                                   x + w, y + h);
    if (_current.pixel_snap)
    {
        rect = PixelSnap(rect);
        //r = PixelSnap(r);
    }

    D2D1_ROUNDED_RECT stroke_rrect = D2D1::RoundedRect(rect, r, r);
    D2D1_ROUNDED_RECT fill_rrect = stroke_rrect;
    if (_current.stroke_width > 0)
    {
        const float stroke_width = std::max(0.0f,
                                            std::min({
                                                0.5f * (stroke_rrect.rect.right - stroke_rrect.rect.left),
                                                0.5f * (stroke_rrect.rect.bottom - stroke_rrect.rect.top),
                                                _current.stroke_width
                                            }));
        fill_rrect.rect.left += stroke_width;
        fill_rrect.rect.top += stroke_width;
        fill_rrect.rect.right -= stroke_width;
        fill_rrect.rect.bottom -= stroke_width;

        const float half_width = 0.5f * stroke_width;
        stroke_rrect.rect.left += half_width;
        stroke_rrect.rect.top += half_width;
        stroke_rrect.rect.right -= half_width;
        stroke_rrect.rect.bottom -= half_width;
    }

    if (_current.HasFill())
    {
        ComPtr<ID2D1Brush> brush;
        if (_current.gradient_brush)
        {
            brush = _current.gradient_brush;
            brush->SetTransform(D2D1::Matrix3x2F::Translation(fill_rrect.rect.left, fill_rrect.rect.top));
        }
        else
        {
            brush = CreateBrush(_current.color);
        }
        _rt->FillRoundedRectangle(fill_rrect, brush.Get());
    }
    if (_current.HasStroke())
    {
        ComPtr<ID2D1Brush> brush = CreateBrush(_current.stroke_color);
        _rt->DrawRoundedRectangle(stroke_rrect, brush.Get(), _current.stroke_width);
    }
}

void Painter::DrawCircle(float center_x, float center_y, float radius)
{
    auto circle = D2D1::Ellipse(D2D1::Point2F(center_x, center_y), radius, radius);

    if (_current.HasFill())
    {
        ComPtr<ID2D1Brush> brush;
        if (_current.gradient_brush)
        {
            brush = _current.gradient_brush;
            //brush->SetTransform(D2D1::Matrix3x2F::Translation(center_xfill_rect.left, fill_rect.top));
        }
        else
        {
            brush = CreateBrush(_current.color);
        }
        _rt->FillEllipse(circle, brush.Get());
    }
    if (_current.HasStroke())
    {
        ComPtr<ID2D1Brush> brush = CreateBrush(_current.stroke_color);
        _rt->DrawEllipse(circle, brush.Get(), _current.stroke_width);
    }
}

void Painter::DrawArc(float center_x, float center_y, float radius, float start_angle, float span_angle)
{
    auto geom = GraphicDeviceD2D::instance()->createPathGeometry();
    if (!geom)
        return;
    start_angle -= 90.0f;
    float start_angle_radians = start_angle * M_PI / 180.f;
    float end_angle_radians = (start_angle + span_angle) * M_PI / 180.f;
    float start_x = center_x + radius * cosf(start_angle_radians);
    float start_y = center_y + radius * sinf(start_angle_radians);
    float end_x = center_x + radius * cosf(end_angle_radians);
    float end_y = center_y + radius * sinf(end_angle_radians);
    ComPtr<ID2D1GeometrySink> gs;
    geom->Open(gs.GetAddressOf());
    gs->BeginFigure(D2D1::Point2F(start_x, start_y), D2D1_FIGURE_BEGIN_HOLLOW);
    D2D1_ARC_SIZE arc_size = span_angle > 180 ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL;
    gs->AddArc(D2D1::ArcSegment(D2D1::Point2F(end_x, end_y), D2D1::SizeF(radius, radius), 0.0f,
                                D2D1_SWEEP_DIRECTION_CLOCKWISE, arc_size));
    gs->EndFigure(D2D1_FIGURE_END_OPEN);
    gs->Close();
    if (_current.HasStroke())
    {
        ComPtr<ID2D1Brush> brush = CreateBrush(_current.stroke_color);
        _rt->DrawGeometry(geom.Get(), brush.Get(), _current.stroke_width);
    }
}

void Painter::DrawTextLayout(float x, float y, const TextLayoutD2D& layout)
{
    D2D1_POINT_2F origin = D2D1::Point2F(x, y);
    if (_current.HasFill())
    {
        ComPtr<ID2D1Brush> brush = CreateBrush(_current.color);
        if (_current.gradient_brush)
        {
            brush = _current.gradient_brush;
            brush->SetTransform(D2D1::Matrix3x2F::Translation(origin.x, origin.y));
        }
        else
        {
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
    D2D1_POINT_2F origin = D2D1::Point2F(x, y);
    if (_current.HasFill())
    {
        ComPtr<ID2D1Brush> brush = CreateBrush(_current.color);
        if (_current.gradient_brush)
        {
            brush = _current.gradient_brush;
            brush->SetTransform(D2D1::Matrix3x2F::Translation(origin.x, origin.y));
        }
        else
        {
            brush = CreateBrush(_current.color);
        }
        _rt->DrawGlyphRun(origin,
                          gr.raw(),
                          brush.Get(),
                          DWRITE_MEASURING_MODE_NATURAL);
    }
}

void Painter::Save()
{
    _state_stack.push_back(_current);
}

void Painter::Restore()
{
    int n = _current.push_clip_rect_count;

    if (!_state_stack.empty())
    {
        _current = _state_stack.back();
        _state_stack.pop_back();
    }
    else
    {
        _current.Reset();
    }

    while (n-- > _current.push_clip_rect_count)
    {
        _rt->PopAxisAlignedClip();
    }
    _rt->SetTransform(_current.transform);
}

ComPtr<ID2D1Brush> Painter::CreateBrush(const style::Color& c)
{
    D2D1_COLOR_F color = D2D1::ColorF(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
    ComPtr<ID2D1SolidColorBrush> brush;
    HRESULT hr = _rt->CreateSolidColorBrush(color, brush.GetAddressOf());
    return brush;
}

D2D1_RECT_F Painter::PixelSnap(const D2D1_RECT_F& rect)
{
    float left = PixelSnap(rect.left);
    float top = PixelSnap(rect.top);
    float right = PixelSnap(rect.right);
    float bottom = PixelSnap(rect.bottom);
    return D2D1::RectF(left, top, right, bottom);
}

D2D1_RECT_F Painter::PixelSnapConservative(const D2D1_RECT_F& rect)
{
    float left = floorf(rect.left * _dpi_scale) / _dpi_scale;
    float top = floorf(rect.top * _dpi_scale) / _dpi_scale;
    float right = ceilf(rect.right * _dpi_scale) / _dpi_scale;
    float bottom = ceilf(rect.bottom * _dpi_scale) / _dpi_scale;
    return D2D1::RectF(left, top, right, bottom);
}

// void PainterImpl::scale(const scene2d::PointF& s)
// {
// }

/*
void PainterImpl::drawBox(const scene2d::RectF& padding_rect, const style::EdgeOffsetF& border_width,
	const scene2d::CornerRadiusF& border_radius, const style::Color& background_color, const style::Color& border_color,
	const graph2d::BitmapInterface* background_image)
{
	auto rect1 = scene2d::RectF::fromLTRB(
		padding_rect.left - border_width.left,
		padding_rect.top - border_width.top,
		padding_rect.right + border_width.right,
		padding_rect.bottom + border_width.bottom);
	float max_border_width = std::max(
		{border_width.left, border_width.top, border_width.right, border_width.bottom});
	p_.SetStrokeWidth(max_border_width);
	p_.SetStrokeColor(border_color);
	p_.SetColor(background_color);
	float max_border_raidus = std::max(
		{border_radius.top_left, border_radius.top_right, border_radius.bottom_right, border_radius.bottom_left});
	if (max_border_raidus > 0.0f)
	{
		p_.DrawRoundedRect(rect1.origin(), rect1.size(), max_border_raidus);
	}
	else
	{
		p_.DrawRect(rect1.origin(), rect1.size());
	}
	if (background_image)
	{
		auto bitmap = static_cast<const BitmapImpl*>(background_image)->d2dBitmap(p_);
		if (bitmap)
		{
			p_.DrawBitmap(bitmap, padding_rect.origin(), padding_rect.size());
		}
	}
}
*/

void PaintPathD2D::moveTo(float x, float y)
{
    updateCheck();

    if (firstp_.has_value())
        gsink_->EndFigure(D2D1_FIGURE_END_OPEN);

    firstp_ = lastp_ = scene2d::PointF(x, y);
    gsink_->BeginFigure(firstp_.value(), D2D1_FIGURE_BEGIN_FILLED);
}

void PaintPathD2D::lineTo(float x, float y)
{
    lastp_ = scene2d::PointF(x, y);
    gsink_->AddLine(lastp_.value());
}

void PaintPathD2D::arcTo(float radius_x, float radius_y, float rotation_degress, graph2d::SweepDirection sweep_dir,
                         graph2d::ArcSize arc_size, float x, float y)
{
    lastp_ = scene2d::PointF(x, y);
    gsink_->AddArc(D2D1_ARC_SEGMENT{
        {x, y},
        {radius_x, radius_y},
        rotation_degress,
        (sweep_dir == graph2d::SweepDirection::SWEEP_DIRECTION_CLOCKWISE)
            ? D2D1_SWEEP_DIRECTION_CLOCKWISE
            : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
        (arc_size == graph2d::ArcSize::ARC_SIZE_SMALL)
            ? D2D1_ARC_SIZE_SMALL
            : D2D1_ARC_SIZE_LARGE
    });
}

void PaintPathD2D::close()
{
    gsink_->EndFigure(D2D1_FIGURE_END_CLOSED);
    firstp_ = absl::nullopt;
}

void PaintPathD2D::addRRect(const scene2d::RRectF& rrect)
{
    // TopLeft
    if (const auto& c = rrect.corner_radius.top_left; !c.isZeros())
    {
        moveTo(rrect.rect.left, rrect.rect.top + c.height);
        arcTo(c.width,
              c.height,
              90.0f,
              graph2d::SweepDirection::SWEEP_DIRECTION_CLOCKWISE,
              graph2d::ArcSize::ARC_SIZE_SMALL,
              rrect.rect.left + c.width,
              rrect.rect.top);
    }
    else
    {
        moveTo(rrect.rect.left, rrect.rect.top);
    }

    // Top
    lineTo(rrect.rect.right - rrect.corner_radius.top_right.width, rrect.rect.top);

    // TopRight
    if (const auto& c = rrect.corner_radius.top_right; !c.isZeros())
    {
        arcTo(c.width,
              c.height,
              90.0f,
              graph2d::SweepDirection::SWEEP_DIRECTION_CLOCKWISE,
              graph2d::ArcSize::ARC_SIZE_SMALL,
              rrect.rect.right,
              rrect.rect.top + c.height);
    }

    // Right
    lineTo(rrect.rect.right, rrect.rect.bottom - rrect.corner_radius.bottom_right.height);

    // BottomRight
    if (const auto& c = rrect.corner_radius.bottom_right; !c.isZeros())
    {
        arcTo(c.width,
              c.height,
              90.0f,
              graph2d::SweepDirection::SWEEP_DIRECTION_CLOCKWISE,
              graph2d::ArcSize::ARC_SIZE_SMALL,
              rrect.rect.right - c.width,
              rrect.rect.bottom);
    }

    // Bottom
    lineTo(rrect.rect.left + rrect.corner_radius.bottom_left.width, rrect.rect.bottom);

    // Bottom-Left
    if (const auto& c = rrect.corner_radius.bottom_left; !c.isZeros())
    {
        arcTo(c.width,
              c.height,
              90.0f,
              graph2d::SweepDirection::SWEEP_DIRECTION_CLOCKWISE,
              graph2d::ArcSize::ARC_SIZE_SMALL,
              rrect.rect.left,
              rrect.rect.bottom - c.height);
    }

    // Left
    close();
}

void PaintPathD2D::updateCheck()
{
    if (!geometry_) reset();
    if (!gsink_)
    {
        if (finalized_)
        {
        TRY_SEALED:
            ComPtr<ID2D1PathGeometry> tpg = GraphicDeviceD2D::instance()->createPathGeometry();
            if (!tpg) return;

            HRESULT hr;
            UINT32 count = 0;
            geometry_->GetFigureCount(&count);

            if (count)
            {
                hr = tpg->Open(gsink_.ReleaseAndGetAddressOf());
                DCHECK(SUCCEEDED(hr));
                if (FAILED(hr)) return;
                hr = geometry_->Stream(gsink_.Get());
                assert(SUCCEEDED(hr));
                if (FAILED(hr)) return;
            }
            geometry_ = tpg;
            finalized_ = false;
        }
        else
        {
            HRESULT hr = geometry_->Open(gsink_.ReleaseAndGetAddressOf());
            //ps->SetFillMode(D2D1_FILL_MODE::D2D1_FILL_MODE_ALTERNATE);
            if (hr == 0x88990001) // object is not in correct state
                goto TRY_SEALED;
            CHECK(SUCCEEDED(hr));
        }
    }
}

void PaintPathD2D::reset()
{
    geometry_ = GraphicDeviceD2D::instance()->createPathGeometry();
    CHECK(geometry_ != nullptr);
}

void PaintPathD2D::finalize()
{
    if (!finalized_)
    {
        if (gsink_)
        {
            if (firstp_.has_value())
            {
                gsink_->EndFigure(D2D1_FIGURE_END_OPEN);
                firstp_ = absl::nullopt;
            }
            HRESULT hr = gsink_->Close();
            CHECK(SUCCEEDED(hr));
            gsink_ = nullptr;
        }
        finalized_ = true;
    }
}

/*
void PainterImpl::drawBoxShadow(const scene2d::RectF& padding_rect, const style::EdgeOffsetF& inset_border_width,
                                const scene2d::CornerRadiusF& border_radius, const graph2d::BoxShadow& box_shadow)
{
    const float dpi_scale = p_.GetDpiScale();;
    if (box_shadow.inset) {
        // Compute extra padding
        float blur_radius = roundf(box_shadow.blur_radius * dpi_scale) / dpi_scale;
        D2D1_RECT_F rect = p_.PixelSnapConservative(
            scene2d::RectF::fromXYWH(blur_radius,
                                     blur_radius,
                                     padding_rect.width() + inset_border_width.left +
                                     inset_border_width.right,
                                     padding_rect.height() + inset_border_width.top +
                                     inset_border_width.bottom));
        scene2d::RRectF rrect = scene2d::RRectF::fromRectRadius(
            scene2d::RectF::fromLTRB(rect.left, rect.top, rect.right, rect.bottom),
            border_radius);

        // Blit the opacity bitmap
        auto dst_rrect = scene2d::RRectF::fromRectRadius(padding_rect, border_radius);
        auto src_origin = scene2d::PointF(
            padding_rect.left - inset_border_width.left - blur_radius + box_shadow.offset_x,
            padding_rect.top - inset_border_width.top - blur_radius + box_shadow.offset_y);

        // Create opacity bitmap brush
        ComPtr<ID2D1Bitmap> opacity_bitmap = makeInsetShadowBitmap(padding_rect,
                                                                   inset_border_width,
                                                                   border_radius,
                                                                   box_shadow);
        ComPtr<ID2D1BitmapBrush> opacity_brush;
        p_._rt->CreateBitmapBrush(opacity_bitmap.Get(), opacity_brush.GetAddressOf());
        opacity_brush->SetTransform(D2D1::Matrix3x2F::Translation(src_origin.x, src_origin.y));

        // Create solid color bitmap brush
        uint32_t color_data[1] = {box_shadow.color.makePremultipliedAlpha().getArgb()};
        auto color_bitmap = GraphicDeviceD2D::instance()
            ->createBitmap(1, 1, 1, color_data, 1, DXGI_FORMAT_B8G8R8A8_UNORM,
                           D2D1_ALPHA_MODE_PREMULTIPLIED);
        ComPtr<ID2D1BitmapBrush> color_brush;
        p_._rt->CreateBitmapBrush(color_bitmap.Get(), color_brush.GetAddressOf());

        // Draw the RRect
        auto path = std::make_unique<PaintPathD2D>();
        path->addRRect(dst_rrect);
        p_._rt->FillGeometry(path->getD2D1PathGeometry(), color_brush.Get(), opacity_brush.Get());
    } else {
        // Compute extra padding
        float blur_radius = roundf(box_shadow.blur_radius * dpi_scale) / dpi_scale;
        D2D1_RECT_F rect = p_.PixelSnapConservative(
            scene2d::RectF::fromXYWH(blur_radius,
                                     blur_radius,
                                     padding_rect.width() + inset_border_width.left +
                                     inset_border_width.right,
                                     padding_rect.height() + inset_border_width.top +
                                     inset_border_width.bottom));
        scene2d::RRectF rrect = scene2d::RRectF::fromRectRadius(
            scene2d::RectF::fromLTRB(rect.left, rect.top, rect.right, rect.bottom),
            border_radius);

        // Blit the opacity bitmap
        auto dst_rect = (D2D1_RECT_F)scene2d::RectF::fromXYWH(
            padding_rect.left - inset_border_width.left - blur_radius + box_shadow.offset_x,
            padding_rect.top - inset_border_width.top - blur_radius + box_shadow.offset_y,
            blur_radius + rrect.width() + blur_radius,
            blur_radius + rrect.height() + blur_radius);
        auto color_brush = p_.CreateBrush(box_shadow.color);
        ComPtr<ID2D1Bitmap> opacity_bitmap = makeOutsetShadowBitmap(padding_rect,
                                                                    inset_border_width,
                                                                    border_radius,
                                                                    box_shadow);
        auto aa_mode = p_._rt->GetAntialiasMode();
        p_._rt->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
        p_._rt->FillOpacityMask(opacity_bitmap.Get(),
                                color_brush.Get(),
                                D2D1_OPACITY_MASK_CONTENT_GRAPHICS,
                                &dst_rect);
        p_._rt->SetAntialiasMode(aa_mode);
    }
}
*/
void PainterImpl::drawBoxShadow(const scene2d::RectF& padding_rect, const style::EdgeOffsetF& inset_border_width,
                                const scene2d::CornerRadiusF& border_radius, const graph2d::BoxShadow& box_shadow)
{
    const float dpi_scale = p_.GetDpiScale();;
    if (box_shadow.inset)
    {
        // Compute extra padding
        float blur_radius = roundf(box_shadow.blur_radius * dpi_scale) / dpi_scale;
        D2D1_RECT_F rect = p_.PixelSnapConservative(
            scene2d::RectF::fromXYWH(blur_radius,
                                     blur_radius,
                                     padding_rect.width() + inset_border_width.left +
                                     inset_border_width.right,
                                     padding_rect.height() + inset_border_width.top +
                                     inset_border_width.bottom));
        scene2d::RRectF rrect = scene2d::RRectF::fromRectRadius(
            scene2d::RectF::fromLTRB(rect.left, rect.top, rect.right, rect.bottom),
            border_radius);

        // Blit the opacity bitmap
        auto dst_rrect = scene2d::RRectF::fromRectRadius(padding_rect, border_radius);
        const float PADDING = PADDING_PIXELS / dpi_scale; // inset border pixmap padding
        auto src_origin = scene2d::PointF(
            padding_rect.left - inset_border_width.left - (blur_radius + PADDING) + box_shadow.offset_x,
            padding_rect.top - inset_border_width.top - (blur_radius + PADDING) + box_shadow.offset_y);

        // Create shadow bitmap
        auto shadow_bmp = makeInsetShadowBitmap(padding_rect,
                                                inset_border_width,
                                                border_radius,
                                                box_shadow);
        ComPtr<ID2D1BitmapBrush> shadow_bmp_brush;
        p_._rt->CreateBitmapBrush(shadow_bmp->d2dBitmap(p_), shadow_bmp_brush.GetAddressOf());
        shadow_bmp_brush->SetTransform(D2D1::Matrix3x2F::Translation(src_origin.x, src_origin.y));

        // Draw the RRect
        auto path = std::make_unique<PaintPathD2D>();
        path->addRRect(dst_rrect);
        p_._rt->FillGeometry(path->getD2D1PathGeometry(), shadow_bmp_brush.Get());
    }
    else
    {
        // Compute extra padding
        float blur_radius = roundf(box_shadow.blur_radius * dpi_scale) / dpi_scale;
        D2D1_RECT_F rect = p_.PixelSnapConservative(
            scene2d::RectF::fromXYWH(blur_radius,
                                     blur_radius,
                                     padding_rect.width() + inset_border_width.left +
                                     inset_border_width.right,
                                     padding_rect.height() + inset_border_width.top +
                                     inset_border_width.bottom));
        scene2d::RRectF rrect = scene2d::RRectF::fromRectRadius(
            scene2d::RectF::fromLTRB(rect.left, rect.top, rect.right, rect.bottom),
            border_radius);

        // Blit the opacity bitmap
        auto dst_rect = scene2d::RectF::fromXYWH(
            padding_rect.left - inset_border_width.left - blur_radius + box_shadow.offset_x,
            padding_rect.top - inset_border_width.top - blur_radius + box_shadow.offset_y,
            blur_radius + rrect.width() + blur_radius,
            blur_radius + rrect.height() + blur_radius);

        absl::optional<style::EdgeOffsetF> expand_edges;
        auto shadow_bmp = makeOutsetShadowBitmap(padding_rect,
                                                 inset_border_width,
                                                 border_radius,
                                                 box_shadow,
                                                 expand_edges);
        if (expand_edges.has_value())
        {
            drawBitmapNine(shadow_bmp.get(), expand_edges.value(), dst_rect);
        }
        else
        {
            drawBitmap(shadow_bmp.get(), dst_rect.origin(), dst_rect.size());
        }
    }
}

void PainterImpl::drawRect(const scene2d::RectF& rect, const graph2d::PaintBrush& brush)
{
    D2D1_RECT_F rc(rect);
    if (brush.style() == graph2d::PAINT_STYLE_FILL)
    {
        if (brush.color().getAlpha() > 0)
        {
            ComPtr<ID2D1Brush> d2d_brush = p_.CreateBrush(brush.color());
            p_._rt->FillRectangle(rc, d2d_brush.Get());
        }
        if (brush.shader())
        {
            const auto* bitmap = (const BitmapFromUrlD2D*)brush.shader();
            auto* d2d_bitmap = bitmap->d2dBitmap(p_);
            ComPtr<ID2D1Brush> d2d_brush = p_.CreateBitmapBrush(d2d_bitmap);
            d2d_brush->SetTransform(D2D1::Matrix3x2F::Translation(rc.left, rc.top));
            p_._rt->FillRectangle(rc, d2d_brush.Get());
        }
    }
    else
    {
        if (brush.color().getAlpha() > 0)
        {
            ComPtr<ID2D1Brush> d2d_brush = p_.CreateBrush(brush.color());
            p_._rt->DrawRectangle(rc, d2d_brush.Get(), brush.strokeWidth());
        }
    }
}

void PainterImpl::drawRRect(const scene2d::RRectF& rrect, const graph2d::PaintBrush& brush)
{
    // Draw rect
    if (rrect.isRect())
        return drawRect(rrect.rect, brush);

    // All corners' radius have same width and height
    if (rrect.corner_radius.isSymmetric())
    {
        const auto& radius = rrect.corner_radius.top_left;

        D2D1_ROUNDED_RECT rc{
            rrect.rect,
            radius.width,
            radius.height,
        };
        if (brush.style() == graph2d::PAINT_STYLE_FILL)
        {
            if (brush.color().getAlpha() > 0)
            {
                ComPtr<ID2D1Brush> d2d_brush = p_.CreateBrush(brush.color());
                p_._rt->FillRoundedRectangle(rc, d2d_brush.Get());
            }
            if (brush.shader())
            {
                const auto* bitmap = (const BitmapFromUrlD2D*)brush.shader();
                auto* d2d_bitmap = bitmap->d2dBitmap(p_);
                ComPtr<ID2D1Brush> d2d_brush = p_.CreateBitmapBrush(d2d_bitmap);
                d2d_brush->SetTransform(D2D1::Matrix3x2F::Translation(rc.rect.left, rc.rect.top));
                p_._rt->FillRoundedRectangle(rc, d2d_brush.Get());
            }
        }
        else
        {
            if (brush.color().getAlpha() > 0)
            {
                ComPtr<ID2D1Brush> d2d_brush = p_.CreateBrush(brush.color());
                p_._rt->DrawRoundedRectangle(rc, d2d_brush.Get(), brush.strokeWidth());
            }
        }
        return;
    }

    // Generate rrect path
    auto path = std::make_unique<PaintPathD2D>();
    {
        // TopLeft
        if (const auto& c = rrect.corner_radius.top_left; !c.isZeros())
        {
            path->moveTo(rrect.rect.left, rrect.rect.top + c.height);
            path->arcTo(c.width,
                        c.height,
                        90.0f,
                        graph2d::SweepDirection::SWEEP_DIRECTION_CLOCKWISE,
                        graph2d::ArcSize::ARC_SIZE_SMALL,
                        rrect.rect.left + c.width,
                        rrect.rect.top);
        }
        else
        {
            path->moveTo(rrect.rect.left, rrect.rect.top);
        }

        // Top
        path->lineTo(rrect.rect.right - rrect.corner_radius.top_right.width, rrect.rect.top);

        // TopRight
        if (const auto& c = rrect.corner_radius.top_right; !c.isZeros())
        {
            path->arcTo(c.width,
                        c.height,
                        90.0f,
                        graph2d::SweepDirection::SWEEP_DIRECTION_CLOCKWISE,
                        graph2d::ArcSize::ARC_SIZE_SMALL,
                        rrect.rect.right,
                        rrect.rect.top + c.height);
        }

        // Right
        path->lineTo(rrect.rect.right, rrect.rect.bottom - rrect.corner_radius.bottom_right.height);

        // BottomRight
        if (const auto& c = rrect.corner_radius.bottom_right; !c.isZeros())
        {
            path->arcTo(c.width,
                        c.height,
                        90.0f,
                        graph2d::SweepDirection::SWEEP_DIRECTION_CLOCKWISE,
                        graph2d::ArcSize::ARC_SIZE_SMALL,
                        rrect.rect.right - c.width,
                        rrect.rect.bottom);
        }

        // Bottom
        path->lineTo(rrect.rect.left + rrect.corner_radius.bottom_left.width, rrect.rect.bottom);

        // Bottom-Left
        if (const auto& c = rrect.corner_radius.bottom_left; !c.isZeros())
        {
            path->arcTo(c.width,
                        c.height,
                        90.0f,
                        graph2d::SweepDirection::SWEEP_DIRECTION_CLOCKWISE,
                        graph2d::ArcSize::ARC_SIZE_SMALL,
                        rrect.rect.left,
                        rrect.rect.bottom - c.height);
        }

        // Left
        path->close();
    }

    drawPath(path.get(), brush);
}

void PainterImpl::drawDRRect(const scene2d::RRectF& outer, const scene2d::RRectF& inner,
                             const graph2d::PaintBrush& brush)
{
    if (brush.style() == graph2d::PAINT_STYLE_STROKE)
    {
        drawRRect(outer, brush);
        drawRRect(inner, brush);
        return;
    }

    auto path = std::make_unique<PaintPathD2D>();
    path->addRRect(outer);
    path->addRRect(inner);
    drawPath(path.get(), brush);
}

void PainterImpl::drawPath(const graph2d::PaintPathInterface* path, const graph2d::PaintBrush& brush)
{
    auto* d2d_path = dynamic_cast<const PaintPathD2D*>(path)->getD2D1PathGeometry();
    if (!d2d_path)
        return;
    UINT32 fc, sc;
    d2d_path->GetFigureCount(&fc);
    d2d_path->GetSegmentCount(&sc);
    if (brush.style() == graph2d::PAINT_STYLE_FILL)
    {
        if (brush.color().getAlpha() > 0)
        {
            ComPtr<ID2D1Brush> d2d_brush = p_.CreateBrush(brush.color());
            p_._rt->FillGeometry(d2d_path, d2d_brush.Get());
        }
        if (brush.shader())
        {
            const auto* bitmap = (const BitmapFromUrlD2D*)brush.shader();
            auto* d2d_bitmap = bitmap->d2dBitmap(p_);
            ComPtr<ID2D1Brush> d2d_brush = p_.CreateBitmapBrush(d2d_bitmap);
            // TODO: drawPath: brush transform
            p_._rt->FillGeometry(d2d_path, d2d_brush.Get());
        }
    }
    else
    {
        if (brush.color().getAlpha() > 0)
        {
            ComPtr<ID2D1Brush> d2d_brush = p_.CreateBrush(brush.color());
            ComPtr<ID2D1StrokeStyle> stroke_style = GraphicDeviceD2D::instance()
                ->createStrokeStyle(brush.strokeCap(), brush.strokeJoin(), brush.strokeMiterLimit());
            p_._rt->DrawGeometry(d2d_path, d2d_brush.Get(), brush.strokeWidth(), stroke_style.Get());
        }
    }
}

graph2d::PaintSurfaceInterface* PainterImpl::surface() const
{
    return surface_;
}

std::shared_ptr<BitmapD2D> PainterImpl::makeOutsetShadowBitmap(const scene2d::RectF& padding_rect,
                                                               const style::EdgeOffsetF& inset_border_width,
                                                               const scene2d::CornerRadiusF& border_radius,
                                                               const graph2d::BoxShadow& box_shadow,
                                                               absl::optional<style::EdgeOffsetF>& expand_edges)
{
    // Compute extra padding
    const float dpi_scale = p_.GetDpiScale();
    const float blur_radius = roundf(box_shadow.blur_radius * dpi_scale) / dpi_scale;
    D2D1_RECT_F rect = p_.PixelSnapConservative(
        scene2d::RectF::fromXYWH(blur_radius,
                                 blur_radius,
                                 padding_rect.width() + inset_border_width.left +
                                 inset_border_width.right,
                                 padding_rect.height() + inset_border_width.top +
                                 inset_border_width.bottom));

    // Check expand
    auto max_border_radius = 1 + std::max({
        border_radius.top_left.width + inset_border_width.left,
        border_radius.top_left.height + inset_border_width.top,
        border_radius.top_right.width + inset_border_width.right,
        border_radius.top_right.height + inset_border_width.top,
        border_radius.bottom_right.width + inset_border_width.right,
        border_radius.bottom_right.height + inset_border_width.bottom,
        border_radius.bottom_left.width + inset_border_width.left,
        border_radius.bottom_left.height + inset_border_width.bottom
    });
    if (rect.right - rect.left >= 2 * max_border_radius
        && rect.bottom - rect.top >= 2 * max_border_radius)
    {
        expand_edges = style::EdgeOffsetF::fromAll(blur_radius + max_border_radius);
        rect = scene2d::RectF::fromXYWH(blur_radius,
                                        blur_radius,
                                        2 * max_border_radius,
                                        2 * max_border_radius);
    }

    scene2d::RRectF rrect = scene2d::RRectF::fromRectRadius(
        scene2d::RectF::fromLTRB(rect.left, rect.top, rect.right, rect.bottom),
        border_radius);

    // Check bitmap cache
    std::string cache_key = absl::StrFormat("box-shadow: outset, blur=%.2f, pixel_size=%.0fx%.0f, color=#%08x",
                                            blur_radius,
                                            (blur_radius + rrect.width() + blur_radius) * dpi_scale,
                                            (blur_radius + rrect.height() + blur_radius) * dpi_scale,
                                            box_shadow.color.getRgba());

    if (surface_)
    {
        auto b = surface_->getCachedBitmap(cache_key);
        if (b)
            return b;
    }

    // Create shadow bitmap
    PaintSurfaceBitmapD2D::CreateInfo ci;
    ci.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    ci.pixel_size.width = (blur_radius + rrect.width() + blur_radius) * dpi_scale;
    ci.pixel_size.height = (blur_radius + rrect.height() + blur_radius) * dpi_scale;
    ci.dpi_scale = dpi_scale;
    auto bmp = PaintSurfaceBitmapD2D::create(ci);
    if (auto bp = bmp->beginPaint())
    {
        bp->clear(style::Color());
        graph2d::PaintBrush brush;
        brush.setColor(box_shadow.color);
        bp->drawRRect(rrect, brush);
        bmp->endPaint();
    }

    // Blur the bitmap
    if (auto mapped = bmp->map())
    {
        UINT stride;
        UINT bmp_size;
        BYTE* bmp_data = nullptr;
        mapped->GetStride(&stride);
        mapped->GetDataPointer(&bmp_size, &bmp_data);
        if (bmp_data)
        {
            JuceImage jimg = JuceImage::fromARGB(ci.pixel_size.width,
                                                 ci.pixel_size.height,
                                                 bmp_data,
                                                 stride);
            applyStackBlur(jimg, blur_radius * dpi_scale);

            // Make the bitmap
            auto shadow_bitmap = std::make_shared<BitmapD2D>(bmp_data, (size_t)ci.pixel_size.width,
                                                             (size_t)ci.pixel_size.height,
                                                             stride, dpi_scale, ci.format,
                                                             D2D1_ALPHA_MODE_PREMULTIPLIED);
            if (surface_ && shadow_bitmap)
                surface_->updateCachedBitmap(cache_key, shadow_bitmap);
            return shadow_bitmap;
        }
    }

    return nullptr;
}

std::shared_ptr<BitmapD2D> PainterImpl::makeInsetShadowBitmap(const scene2d::RectF& padding_rect,
                                                              const style::EdgeOffsetF& inset_border_width,
                                                              const scene2d::CornerRadiusF& border_radius,
                                                              const graph2d::BoxShadow& box_shadow)
{
    // Compute extra padding
    const float dpi_scale = p_.GetDpiScale();
    const float PADDING = PADDING_PIXELS / dpi_scale;
    float blur_radius = roundf(box_shadow.blur_radius * dpi_scale) / dpi_scale;
    D2D1_RECT_F rect = p_.PixelSnapConservative(
        scene2d::RectF::fromXYWH(blur_radius + PADDING,
                                 blur_radius + PADDING,
                                 padding_rect.width() + inset_border_width.left +
                                 inset_border_width.right,
                                 padding_rect.height() + inset_border_width.top +
                                 inset_border_width.bottom));
    scene2d::RRectF rrect = scene2d::RRectF::fromRectRadius(
        scene2d::RectF::fromLTRB(rect.left, rect.top, rect.right, rect.bottom),
        border_radius);

    // Check bitmap cache
    std::string cache_key = absl::StrFormat("box-shadow: inset, blur=%.2f, pixel_size=%.0fx%.0f, color=#%08x",
                                            blur_radius,
                                            (blur_radius + rrect.width() + blur_radius) * dpi_scale,
                                            (blur_radius + rrect.height() + blur_radius) * dpi_scale,
                                            box_shadow.color.getRgba());

    if (surface_)
    {
        auto b = surface_->getCachedBitmap(cache_key);
        if (b)
            return b;
    }

    // Create shadow bitmap
    PaintSurfaceBitmapD2D::CreateInfo ci;
    ci.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    ci.pixel_size.width = roundl((blur_radius + PADDING + rrect.width() + blur_radius + PADDING) * dpi_scale);
    ci.pixel_size.height = roundl((blur_radius + PADDING + rrect.height() + blur_radius + PADDING) * dpi_scale);
    ci.dpi_scale = dpi_scale;
    auto bmp = PaintSurfaceBitmapD2D::create(ci);

    // Draw the border
    if (auto bp = bmp->beginPaint())
    {
        bp->clear(style::Color());
        graph2d::PaintBrush brush;
        brush.setColor(box_shadow.color);
        auto outer = scene2d::RRectF::fromRectRadius(
            scene2d::RectF::fromOriginSize(scene2d::PointF(), ci.pixel_size / dpi_scale),
            scene2d::CornerRadiusF());
        bp->drawDRRect(outer, rrect, brush);
        bmp->endPaint();
    }

    // Blur the bitmap
    if (auto mapped = bmp->map())
    {
        UINT stride;
        UINT bmp_size;
        BYTE* bmp_data = nullptr;
        mapped->GetStride(&stride);
        mapped->GetDataPointer(&bmp_size, &bmp_data);
        if (bmp_data)
        {
            JuceImage jimg = JuceImage::fromARGB(ci.pixel_size.width,
                                                 ci.pixel_size.height,
                                                 bmp_data,
                                                 stride);
            applyStackBlur(jimg, blur_radius * dpi_scale);

            // Make the bitmap
            auto shadow_bitmap = std::make_shared<BitmapD2D>(bmp_data, (size_t)ci.pixel_size.width,
                                                             (size_t)ci.pixel_size.height,
                                                             stride, dpi_scale, ci.format,
                                                             D2D1_ALPHA_MODE_PREMULTIPLIED);
            if (surface_ && shadow_bitmap)
                surface_->updateCachedBitmap(cache_key, shadow_bitmap);
            return shadow_bitmap;
        }
    }

    return nullptr;
}

void Painter::Translate(const scene2d::PointF& offset)
{
    //_current.offset += offset;
    //LOG(INFO) << "Translate current: " << _current.offset;
    _current.transform = _current.transform * D2D1::Matrix3x2F::Translation(offset.x, offset.y);
    _rt->SetTransform(_current.transform);
}

void Painter::SetTranslation(const scene2d::PointF& abs_offset)
{
    //_current.offset = abs_offset;
    //LOG(INFO) << "SetTranslation current: " << _current.offset;
    _current.transform = D2D1::Matrix3x2F::Translation(abs_offset.x, abs_offset.y);
    _rt->SetTransform(_current.transform);
}

void Painter::Rotate(float degrees, const scene2d::PointF& center)
{
    auto c = _current.transform.TransformPoint(D2D1::Point2F(center.x, center.y));
    _current.transform = _current.transform * D2D1::Matrix3x2F::Rotation(degrees, c);
    _rt->SetTransform(_current.transform);
}

void Painter::SetRotation(float degrees, const scene2d::PointF& center)
{
    auto c = _current.transform.TransformPoint(D2D1::Point2F(center.x, center.y));
    _current.transform = D2D1::Matrix3x2F::Rotation(degrees, c);
    _rt->SetTransform(_current.transform);
}

void Painter::ClipRect(const scene2d::PointF& origin, const scene2d::DimensionF& size)
{
    D2D1_RECT_F rect = D2D1::RectF(origin.x, origin.y, origin.x + size.width, origin.y + size.height);
    if (_current.pixel_snap)
        rect = PixelSnapConservative(rect);
    _rt->PushAxisAlignedClip(rect, D2D1_ANTIALIAS_MODE_ALIASED);
    ++_current.push_clip_rect_count;
}

ComPtr<ID2D1Bitmap> Painter::CreateBitmap(const BitmapSubItem& item)
{
    ComPtr<ID2D1Bitmap> bitmap;
    HRESULT hr;
    D2D1_BITMAP_PROPERTIES bitmap_props
        = D2D1::BitmapProperties(D2D1::PixelFormat(),
                                 item.dpi_scale.x * USER_DEFAULT_SCREEN_DPI,
                                 item.dpi_scale.y * USER_DEFAULT_SCREEN_DPI);
    hr = _rt->CreateBitmapFromWicBitmap(
        item.converter.Get(), bitmap_props, bitmap.GetAddressOf());
    if (FAILED(hr))
    {
        LOG(ERROR) << "Painter::CreateBitmap failed, hr=" << std::hex << hr;
    }
    return bitmap;
}

void Painter::DrawBitmap(ID2D1Bitmap* bitmap, float x, float y, float w, float h)
{
    D2D1_RECT_F rect = D2D1::RectF(x, y, x + w, y + h);
    if (_current.pixel_snap)
        rect = PixelSnap(rect);
    _rt->DrawBitmap(bitmap, rect, 1.0);
}

void Painter::DrawBitmapRect(ID2D1Bitmap* bitmap, const scene2d::RectF& src_rect, const scene2d::RectF& dst_rect)
{
    D2D1_RECT_F d2d_dst_rect = dst_rect;
    if (_current.pixel_snap)
        d2d_dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, d2d_dst_rect,
                    1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                    src_rect);
}

void Painter::DrawScale9Bitmap(ID2D1Bitmap* bitmap, float x, float y, float w, float h,
                               float src_margin, float dst_margin)
{
    D2D1_SIZE_F bitmap_size = bitmap->GetSize();
    D2D1_RECT_F src_rect, dst_rect;
    // TopLeft
    src_rect = D2D1::RectF(0,
                           0,
                           src_margin,
                           src_margin);
    dst_rect = D2D1::RectF(x, y, x + dst_margin, y + dst_margin);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);
    // Top
    src_rect = D2D1::RectF(src_margin,
                           0,
                           bitmap_size.width - src_margin,
                           src_margin);
    dst_rect = D2D1::RectF(x + dst_margin, y, x + w - dst_margin, y + dst_margin);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);
    // TopRight
    src_rect = D2D1::RectF(bitmap_size.width - src_margin,
                           0,
                           bitmap_size.width,
                           src_margin);
    dst_rect = D2D1::RectF(x + w - dst_margin, y, x + w, y + dst_margin);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);


    // Left
    src_rect = D2D1::RectF(0,
                           src_margin,
                           src_margin,
                           bitmap_size.height - src_margin);
    dst_rect = D2D1::RectF(x, y + dst_margin, x + dst_margin, y + h - dst_margin);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);
    // Center
    src_rect = D2D1::RectF(src_margin,
                           src_margin,
                           bitmap_size.width - src_margin,
                           bitmap_size.height - src_margin);
    dst_rect = D2D1::RectF(x + dst_margin, y + dst_margin, x + w - dst_margin, y + h - dst_margin);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);
    // Right
    src_rect = D2D1::RectF(bitmap_size.width - src_margin,
                           src_margin,
                           bitmap_size.width,
                           bitmap_size.height - src_margin);
    dst_rect = D2D1::RectF(x + w - dst_margin,
                           y + dst_margin,
                           x + w,
                           y + h - dst_margin);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);

    // BottomLeft
    src_rect = D2D1::RectF(0,
                           bitmap_size.height - src_margin,
                           src_margin,
                           bitmap_size.height);
    dst_rect = D2D1::RectF(x,
                           y + h - dst_margin,
                           x + dst_margin,
                           y + h);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);
    // Bottom
    src_rect = D2D1::RectF(src_margin,
                           bitmap_size.height - src_margin,
                           bitmap_size.width - src_margin,
                           bitmap_size.height);
    dst_rect = D2D1::RectF(x + dst_margin,
                           y + h - dst_margin,
                           x + w - dst_margin,
                           y + h);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);
    // BottomRight
    src_rect = D2D1::RectF(bitmap_size.width - src_margin,
                           bitmap_size.height - src_margin,
                           bitmap_size.width,
                           bitmap_size.height);
    dst_rect = D2D1::RectF(x + w - dst_margin,
                           y + h - dst_margin,
                           x + w,
                           y + h);
    if (_current.pixel_snap)
        dst_rect = PixelSnap(dst_rect);
    _rt->DrawBitmap(bitmap, dst_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);
}

ComPtr<ID2D1LinearGradientBrush> Painter::CreateLinearGradientBrush_Logo()
{
    HRESULT hr;
    ComPtr<ID2D1LinearGradientBrush> brush;
    ComPtr<ID2D1GradientStopCollection> pGradientStops;
    style::Color c0 = style::Color::fromString(/*"#FF8833"*/"#FF6A00");
    style::Color c1 = style::Color::fromString("#FF3F00");
    //c0 = RED;
    //c1 = BLUE;
    D2D1_GRADIENT_STOP gradientStops[2];
    gradientStops[0].color = D2D1::ColorF(c0.getRed(), c0.getGreen(), c0.getBlue(), c0.getAlpha());
    gradientStops[0].position = 0.0f;
    gradientStops[1].color = D2D1::ColorF(c1.getRed(), c1.getGreen(), c1.getBlue(), c1.getAlpha());
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
    if (FAILED(hr))
    {
        LOG(ERROR) << "Create LinearGradientBrush failed, hr=" << std::hex << hr;
    }
    return brush;
}

ComPtr<ID2D1RadialGradientBrush> Painter::CreateRadialGradientBrush_Highlight()
{
    HRESULT hr;
    ComPtr<ID2D1RadialGradientBrush> brush;
    ComPtr<ID2D1GradientStopCollection> pGradientStops;
    style::Color c0 = style::Color::fromString("#FFFFFF40");
    style::Color c1 = style::Color::fromString("#FFFFFF00");
    D2D1_GRADIENT_STOP gradientStops[2];
    gradientStops[0].color = D2D1::ColorF(c0.getRed(), c0.getGreen(), c0.getBlue(), c0.getAlpha());
    gradientStops[0].position = 0.0f;
    gradientStops[1].color = D2D1::ColorF(c1.getRed(), c1.getGreen(), c1.getBlue(), c1.getAlpha());
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
    if (FAILED(hr))
    {
        LOG(ERROR) << "Create RadialGradientBrush failed, hr=" << std::hex << hr;
    }
    return brush;
}

ComPtr<ID2D1BitmapBrush> Painter::CreateBitmapBrush(ID2D1Bitmap* bitmap)
{
    if (!bitmap)
        return nullptr;

    HRESULT hr;
    ComPtr<ID2D1BitmapBrush> brush;
    hr = _rt->CreateBitmapBrush(bitmap, brush.GetAddressOf());
    if (FAILED(hr))
    {
        LOG(ERROR) << "Create BitmapBrush failed, hr=" << std::hex << hr;
    }
    return brush;
}

ComPtr<ID2D1Bitmap> Painter::CreateSharedBitmap(IDXGISurface* surface, float dpi_scale, D2D1_ALPHA_MODE alpha_mode)
{
    if (!surface)
        return nullptr;

    HRESULT hr;
    ComPtr<ID2D1Bitmap> bitmap;
    DXGI_SURFACE_DESC desc;
    surface->GetDesc(&desc);
    auto props = D2D1::BitmapProperties(D2D1::PixelFormat(desc.Format, alpha_mode),
                                        USER_DEFAULT_SCREEN_DPI * dpi_scale, USER_DEFAULT_SCREEN_DPI * dpi_scale);
    hr = _rt->CreateSharedBitmap(IID_IDXGISurface, surface, &props, bitmap.GetAddressOf());
    if (FAILED(hr))
    {
        LOG(ERROR) << "CreateSharedBitmap failed, hr=" << std::hex << hr;
    }
    return bitmap;
}

void Painter::SetBrush(ComPtr<ID2D1Brush> brush)
{
    _current.gradient_brush = brush;
}

} // namespace windows::graphics
