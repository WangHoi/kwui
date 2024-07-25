#include "PainterD2D.h"
#include "base/log.h"
#define _USE_MATH_DEFINES 1
#include <math.h>

namespace windows::graphics {

Painter::Painter(ID2D1RenderTarget* rt, const scene2d::PointF& mouse_pos)
	: _rt(rt), _mouse_position(mouse_pos) {
	_rt->GetDpi(&_dpi_scale, &_dpi_scale);
	_dpi_scale /= USER_DEFAULT_SCREEN_DPI;
}

void Painter::Clear(const style::Color& c) {
	D2D1_COLOR_F color = D2D1::ColorF(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
	_rt->Clear(color);
}
void Painter::SetColor(const style::Color& c) {
	_current.color = c;
}
void Painter::SetStrokeColor(const style::Color& c) {
	_current.stroke_color = c;
}
void Painter::SetStrokeWidth(float w) {
	_current.stroke_width = w;
}
void Painter::DrawRect(float x, float y, float w, float h) {
	D2D1_RECT_F stroke_rect = D2D1::RectF(x, y,
		x + w, y + h);
	if (_current.pixel_snap)
		stroke_rect = PixelSnap(stroke_rect);

	D2D1_RECT_F fill_rect = stroke_rect;
	if (_current.stroke_width > 0) {
		const float stroke_width = std::max(0.0f,
			std::min({ 0.5f * (stroke_rect.right - stroke_rect.left),
				0.5f * (stroke_rect.bottom - stroke_rect.top),
				_current.stroke_width }));
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

	if (_current.HasFill()) {
		ComPtr<ID2D1Brush> brush;
		if (_current.gradient_brush) {
			brush = _current.gradient_brush;
			brush->SetTransform(D2D1::Matrix3x2F::Translation(fill_rect.left, fill_rect.top));
		} else {
			brush = CreateBrush(_current.color);
		}
		_rt->FillRectangle(fill_rect, brush.Get());
	}
	if (_current.HasStroke()) {
		ComPtr<ID2D1Brush> brush = CreateBrush(_current.stroke_color);
		_rt->DrawRectangle(stroke_rect, brush.Get(), _current.stroke_width);
	}
}
void Painter::DrawRoundedRect(float x, float y, float w, float h, float r) {
	D2D1_RECT_F rect = D2D1::RectF(x, y,
		x + w, y + h);
	if (_current.pixel_snap) {
		rect = PixelSnap(rect);
		//r = PixelSnap(r);
	}

	D2D1_ROUNDED_RECT stroke_rrect = D2D1::RoundedRect(rect, r, r);
	D2D1_ROUNDED_RECT fill_rrect = stroke_rrect;
	if (_current.stroke_width > 0) {
		const float stroke_width = std::max(0.0f,
			std::min({ 0.5f * (stroke_rrect.rect.right - stroke_rrect.rect.left),
				0.5f * (stroke_rrect.rect.bottom - stroke_rrect.rect.top),
				_current.stroke_width }));
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

	if (_current.HasFill()) {
		ComPtr<ID2D1Brush> brush;
		if (_current.gradient_brush) {
			brush = _current.gradient_brush;
			brush->SetTransform(D2D1::Matrix3x2F::Translation(fill_rrect.rect.left, fill_rrect.rect.top));
		} else {
			brush = CreateBrush(_current.color);
		}
		_rt->FillRoundedRectangle(fill_rrect, brush.Get());
	}
	if (_current.HasStroke()) {
		ComPtr<ID2D1Brush> brush = CreateBrush(_current.stroke_color);
		_rt->DrawRoundedRectangle(stroke_rrect, brush.Get(), _current.stroke_width);
	}
}
void Painter::DrawCircle(float center_x, float center_y, float radius)
{
	auto circle = D2D1::Ellipse(D2D1::Point2F(center_x, center_y), radius, radius);

	if (_current.HasFill()) {
		ComPtr<ID2D1Brush> brush;
		if (_current.gradient_brush) {
			brush = _current.gradient_brush;
			//brush->SetTransform(D2D1::Matrix3x2F::Translation(center_xfill_rect.left, fill_rect.top));
		} else {
			brush = CreateBrush(_current.color);
		}
		_rt->FillEllipse(circle, brush.Get());
	}
	if (_current.HasStroke()) {
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
	gs->AddArc(D2D1::ArcSegment(D2D1::Point2F(end_x, end_y), D2D1::SizeF(radius, radius), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, arc_size));
	gs->EndFigure(D2D1_FIGURE_END_OPEN);
	gs->Close();
	if (_current.HasStroke()) {
		ComPtr<ID2D1Brush> brush = CreateBrush(_current.stroke_color);
		_rt->DrawGeometry(geom.Get(), brush.Get(), _current.stroke_width);
	}
}
void Painter::DrawTextLayout(float x, float y, const TextLayoutD2D& layout) {
	D2D1_POINT_2F origin = D2D1::Point2F(x, y);
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
	D2D1_POINT_2F origin = D2D1::Point2F(x, y);
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
	_rt->SetTransform(_current.transform);
}
ComPtr<ID2D1Brush> Painter::CreateBrush(const style::Color& c) {
	D2D1_COLOR_F color = D2D1::ColorF(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());
	ComPtr<ID2D1SolidColorBrush> brush;
	HRESULT hr = _rt->CreateSolidColorBrush(color, brush.GetAddressOf());
	return brush;
}
D2D1_RECT_F Painter::PixelSnap(const D2D1_RECT_F& rect) {
	float left = PixelSnap(rect.left);
	float top = PixelSnap(rect.top);
	float right = PixelSnap(rect.right);
	float bottom = PixelSnap(rect.bottom);
	return D2D1::RectF(left, top, right, bottom);
}
D2D1_RECT_F Painter::PixelSnapConservative(const D2D1_RECT_F& rect) {
	float left = floorf(rect.left * _dpi_scale) / _dpi_scale;
	float top = floorf(rect.top * _dpi_scale) / _dpi_scale;
	float right = ceilf(rect.right * _dpi_scale) / _dpi_scale;
	float bottom = ceilf(rect.bottom * _dpi_scale) / _dpi_scale;
	return D2D1::RectF(left, top, right, bottom);
}
void Painter::Translate(const scene2d::PointF& offset) {
	//_current.offset += offset;
	//LOG(INFO) << "Translate current: " << _current.offset;
	_current.transform = _current.transform * D2D1::Matrix3x2F::Translation(offset.x, offset.y);
	_rt->SetTransform(_current.transform);
}
void Painter::SetTranslation(const scene2d::PointF& abs_offset) {
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
void Painter::PushClipRect(const scene2d::PointF& origin, const scene2d::DimensionF& size) {
	D2D1_RECT_F rect = D2D1::RectF(origin.x, origin.y, origin.x + size.width, origin.y + size.height);
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
	D2D1_RECT_F rect = D2D1::RectF(x, y, x + w, y + h);
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

ComPtr<ID2D1LinearGradientBrush> Painter::CreateLinearGradientBrush_Logo() {
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
	if (FAILED(hr)) {
		LOG(ERROR) << "Create LinearGradientBrush failed, hr=" << std::hex << hr;
	}
	return brush;
}
ComPtr<ID2D1RadialGradientBrush> Painter::CreateRadialGradientBrush_Highlight() {
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
	if (FAILED(hr)) {
		LOG(ERROR) << "Create RadialGradientBrush failed, hr=" << std::hex << hr;
	}
	return brush;
}
void Painter::SetBrush(ComPtr<ID2D1Brush> brush) {
	_current.gradient_brush = brush;
}

NativeBitmap Painter::createNativeBitmap(float width, float height)
{
	auto GD = GraphicDeviceD2D::instance();
	HRESULT hr;
	ComPtr<ID3D11Texture2D> tex;
	ComPtr<ID2D1Bitmap1> bitmap;
	hr = GD->createNativeBitmap(width, height, tex, bitmap);
	if (FAILED(hr)) return NativeBitmap();

	NativeBitmap b;
	b.width = width;
	b.height = height;
	b.d3d_tex = tex;
	b.d2d_bitmap = bitmap;
	return b;
}

} // namespace windows::graphics
