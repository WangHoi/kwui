#pragma once

#include "scene2d/geom_types.h"
#include "scene2d/Control.h"
#include "graph2d/Painter.h"
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
    // draw border rect with inset border width
    void DrawRect(float x, float y, float w, float h);
    void DrawRect(const scene2d::PointF& origin, const scene2d::DimensionF& size) { DrawRect(origin.x, origin.y, size.width, size.height); }
    void DrawRoundedRect(float x, float y, float w, float h, float r);
    void DrawRoundedRect(const scene2d::PointF& origin, const scene2d::DimensionF& size, float r) { DrawRoundedRect(origin.x, origin.y, size.width, size.height, r); }
    void DrawTextLayout(const scene2d::PointF& origin, const TextLayout& layout) { DrawTextLayout(origin.x, origin.y, layout); }
    void DrawTextLayout(float x, float y, const TextLayout& layout);
    void DrawGlyphRun(const scene2d::PointF& origin, const GlyphRun& gr) { DrawGlyphRun(origin.x, origin.y, gr); }
    void DrawGlyphRun(float x, float y, const GlyphRun& gr);
    void Translate(float x, float y) { Translate({ x, y }); }
    void Translate(const scene2d::PointF& offset);
    void SetTranslation(const scene2d::PointF& abs_offset);
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

class PainterImpl : public graph2d::PainterInterface
{
public:
    PainterImpl(graphics::Painter& p)
        : p_(p) {}
    static inline graphics::Painter& unwrap(graph2d::PainterInterface& pi)
    {
        return ((PainterImpl&)pi).p_;
    }
    void save() override
    {
        p_.Save();
    }
    void restore() override
    {
        p_.Restore();
    }
    void setTranslation(const scene2d::PointF& offset, bool combine) override
    {
        if (combine)
            p_.Translate(offset);
        else
            p_.SetTranslation(offset);
    }
    void pushClipRect(const scene2d::PointF& origin, const scene2d::DimensionF& size) override
    {
        p_.PushClipRect(origin, size);
    }
    void popClipRect() override
    {
        p_.PopClipRect();
    }
    void drawBox(const scene2d::RectF& padding_rect,
        const style::EdgeOffsetF& border,
        const style::Value& background_color,
        const style::Value& border_color) override
    {
        auto rect1 = scene2d::RectF::fromLTRB(
            padding_rect.left - border.left,
            padding_rect.top - border.top,
            padding_rect.right + border.right,
            padding_rect.bottom + border.bottom);
        auto border_width = std::max(
            { border.left, border.top, border.right, border.bottom });
        p_.SetStrokeWidth(border_width);
        p_.SetStrokeColor(get_color(border_color));
        p_.SetColor(get_color(background_color));
        p_.DrawRect(rect1.origin(), rect1.size());
    }
    void drawGlyphRun(const scene2d::PointF& pos, const graph2d::GlyphRunInterface* gr, const style::Value& color) override
    {
        p_.SetColor(get_color(color));
        auto glyph_run = (graphics::GlyphRun*)gr;
        p_.DrawGlyphRun(pos, *glyph_run);
    }
    void drawControl(const scene2d::RectF& rect, scene2d::Control* control) override
    {
        control->onPaint(*this, rect);
    }
private:
    static graphics::Color get_color(const style::Value& v) {
        if (v.isAuto())
            return NO_COLOR;
        if (v.unit == style::ValueUnit::HexColor) {
            return graphics::Color::FromString(v.string_val);
        } else if (v.unit == style::ValueUnit::Keyword) {
            return graphics::Color::FromString(v.keyword_val.c_str());
        }
        return NO_COLOR;
    }
    graphics::Painter& p_;
};

} // namespace graphics
} // namespace windows
