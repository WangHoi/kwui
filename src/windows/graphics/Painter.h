#pragma once

#include "scene2d/geom_types.h"
#include "windows/windows_header.h"
#include "Color.h"
#include "TextLayout.h"
#include "TextFlow.h"
#include "GraphicDevice.h"
#include <vector>

namespace windows {
namespace graphics {

class Painter {
public:
	Painter(ID2D1RenderTarget* rt, const scene2d::PointF& mouse_pos);
    inline scene2d::PointF GetMousePosition() const { return _mouse_position; }
	inline float GetDpiScale() const { return _dpi_scale; }
    void Clear(const Color& c);
    void SetColor(const Color& c);
    void SetStrokeColor(const Color& c);
    void SetStrokeWidth(float w);
    void DrawRect(float x, float y, float w, float h);
    void DrawRect(const scene2d::PointF& origin, const scene2d::DimensionF& size) { DrawRect(origin.x, origin.y, size.width, size.height); }
    void DrawRoundedRect(float x, float y, float w, float h, float r);
    void DrawRoundedRect(const scene2d::PointF& origin, const scene2d::DimensionF& size, float r) { DrawRoundedRect(origin.x, origin.y, size.width, size.height, r); }
    void DrawTextLayout(const scene2d::PointF& origin, const TextLayout& layout) { DrawTextLayout(origin.x, origin.y, layout); }
    void DrawTextLayout(float x, float y, const TextLayout& layout);
    void DrawGlyphRun(const scene2d::PointF& origin, const GlyphRun& gr) { DrawGlyphRun(origin.x, origin.y, gr); }
    void DrawGlyphRun(float x, float y, const GlyphRun& gr);
    void Translate(float x, float y) { Translate({ x, y }); }
    void Translate(const scene2d::PointF& offset) { _current.offset += offset; }
    void SetTranslation(const scene2d::PointF& abs_offset) { _current.offset = abs_offset; }
    const scene2d::PointF& GetAccumTranslate() const { return _current.offset; }
    void PushClipRect(const scene2d::PointF& origin, const scene2d::DimensionF& size);
    void PopClipRect();
    void Save();
    void Restore();
	ComPtr<ID2D1Bitmap> CreateBitmap(const BitmapSubItem& item);
	inline void DrawBitmap(ID2D1Bitmap* bitmap, const scene2d::PointF& origin, const scene2d::DimensionF& size) {
		DrawBitmap(bitmap, origin.x, origin.y, size.width, size.height);
	}
	void DrawBitmap(ID2D1Bitmap* bitmap, float x, float y, float w, float h);
	void DrawScale9Bitmap(ID2D1Bitmap* bitmap, float x, float y, float w, float h,
						  float src_margin, float dst_margin);
    ComPtr<ID2D1LinearGradientBrush> CreateLinearGradientBrush_Logo();
    ComPtr<ID2D1RadialGradientBrush> CreateRadialGradientBrush_Highlight();
    void SetBrush(ComPtr<ID2D1Brush> brush);

private:
    ComPtr<ID2D1Brush> CreateBrush(const Color& c);
    inline float Painter::PixelSnap(float x) {
        return roundf(x * _dpi_scale) / _dpi_scale;
    }
    inline D2D1_POINT_2F Painter::PixelSnap(const D2D1_POINT_2F& p) {
        return D2D1::Point2F(PixelSnap(p.x), PixelSnap(p.y));
    }
    D2D1_RECT_F PixelSnap(const D2D1_RECT_F& rect);
    D2D1_RECT_F PixelSnapConservative(const D2D1_RECT_F& rect);

    struct State {
        scene2d::PointF offset;
        Color color;
        Color stroke_color;
        float stroke_width;
        bool pixel_snap;
        ComPtr<ID2D1Brush> gradient_brush;

        State() {
            Reset();
        }
        inline void Reset() {
            offset = scene2d::PointF::fromZeros();
            color = NO_COLOR;
            stroke_color = NO_COLOR;
            stroke_width = 0.0f;
            pixel_snap = true;
            gradient_brush = nullptr;
        }
        inline bool HasFill() const {
            return color.GetAlpha() != 0 || gradient_brush != nullptr;
        }
        inline bool HasStroke() const {
            return stroke_width != 0 && stroke_color.GetAlpha() != 0;
        }
    };
    ID2D1RenderTarget *_rt;
    scene2d::PointF _mouse_position;
	float _dpi_scale;
    State _current;
    std::vector<State> _state_stack;
};

} // namespace graphics
} // namespace windows
