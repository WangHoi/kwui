#pragma once

#include "scene2d/geom_types.h"
#include "scene2d/Control.h"
#include "graph2d/Painter.h"
#include "graph2d/Bitmap.h"
#include "windows/windows_header.h"
#include "TextLayout.h"
#include "TextFlowD2D.h"
#include "GraphicDevice.h"
#include <vector>

namespace windows {
namespace graphics {

class Painter {
public:
	Painter(ID2D1RenderTarget* rt, const scene2d::PointF& mouse_pos);
    inline scene2d::PointF GetMousePosition() const { return _mouse_position; }
	inline float GetDpiScale() const { return _dpi_scale; }
    void Clear(const style::Color& c);
    void SetColor(const style::Color& c);
    void SetStrokeColor(const style::Color& c);
    void SetStrokeWidth(float w);
    // draw border rect with inset border width
    void DrawRect(float x, float y, float w, float h);
    void DrawRect(const scene2d::PointF& origin, const scene2d::DimensionF& size) { DrawRect(origin.x, origin.y, size.width, size.height); }
    void DrawRoundedRect(float x, float y, float w, float h, float r);
    void DrawRoundedRect(const scene2d::PointF& origin, const scene2d::DimensionF& size, float r) { DrawRoundedRect(origin.x, origin.y, size.width, size.height, r); }
    void DrawCircle(float center_x, float center_y, float radius);
    void DrawCircle(const scene2d::PointF& center, float radius) { DrawCircle(center.x, center.y, radius); }
    void DrawArc(float center_x, float center_y, float radius, float start_angle, float span_angle);
    void DrawArc(const scene2d::PointF& center, float radius, float start_angle, float span_angle) { DrawArc(center.x, center.y, radius, start_angle, span_angle); }
    void DrawTextLayout(const scene2d::PointF& origin, const TextLayout& layout) { DrawTextLayout(origin.x, origin.y, layout); }
    void DrawTextLayout(float x, float y, const TextLayout& layout);
    void DrawGlyphRun(const scene2d::PointF& origin, const GlyphRun& gr) { DrawGlyphRun(origin.x, origin.y, gr); }
    void DrawGlyphRun(float x, float y, const GlyphRun& gr);
    void Translate(float x, float y) { Translate({ x, y }); }
    void Translate(const scene2d::PointF& offset);
    void SetTranslation(const scene2d::PointF& abs_offset);
    void Rotate(float degrees, const scene2d::PointF& center);
    void SetRotation(float degrees, const scene2d::PointF& center);
    //const scene2d::PointF& GetAccumTranslate() const { return _current.offset; }
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
    ComPtr<ID2D1Brush> CreateBrush(const style::Color& c);
    inline float Painter::PixelSnap(float x) {
        return roundf(x * _dpi_scale) / _dpi_scale;
    }
    inline D2D1_POINT_2F Painter::PixelSnap(const D2D1_POINT_2F& p) {
        return D2D1::Point2F(PixelSnap(p.x), PixelSnap(p.y));
    }
    D2D1_RECT_F PixelSnap(const D2D1_RECT_F& rect);
    D2D1_RECT_F PixelSnapConservative(const D2D1_RECT_F& rect);

    struct State {
        //scene2d::PointF offset;
        D2D1::Matrix3x2F transform;
        style::Color color;
        style::Color stroke_color;
        float stroke_width;
        bool pixel_snap;
        ComPtr<ID2D1Brush> gradient_brush;

        State() {
            Reset();
        }
        inline void Reset() {
            //offset = scene2d::PointF::fromZeros();
            transform = D2D1::Matrix3x2F::Identity();
            color = style::Color();
            stroke_color = style::Color();
            stroke_width = 0.0f;
            pixel_snap = true;
            gradient_brush = nullptr;
        }
        inline bool HasFill() const {
            return color.getAlpha() != 0 || gradient_brush != nullptr;
        }
        inline bool HasStroke() const {
            return stroke_width != 0 && stroke_color.getAlpha() != 0;
        }
    };
    ID2D1RenderTarget *_rt;
    scene2d::PointF _mouse_position;
	float _dpi_scale;
    State _current;
    std::vector<State> _state_stack;
};

class BitmapImpl : public graph2d::BitmapInterface
{
public:
    BitmapImpl(const std::string& url)
        : url_(url)
    {
    }
    const std::string& url() const override
    {
        return url_;
    }
    scene2d::DimensionF size() override
    {
        if (!bitmap_) {
            BitmapSubItem item = GraphicDevice::instance()
                ->getBitmap(url_, 1.0f);
            if (item) {
                UINT w, h;
                item.frame->GetSize(&w, &h);
                return scene2d::DimensionF((float)w, (float)h);
            } else {
                return scene2d::DimensionF();
            }
        }
        D2D1_SIZE_U ps = bitmap_->GetPixelSize();
        return scene2d::DimensionF(ps.width, ps.height);
    }
    ID2D1Bitmap* d2dBitmap(Painter& p) const
    {
        if (!bitmap_) {
            BitmapSubItem item = GraphicDevice::instance()
                ->getBitmap(url_, p.GetDpiScale());
            if (item)
                bitmap_ = p.CreateBitmap(item);
        }

        return bitmap_.Get();
    }
private:
    std::string url_; // utf-8
    mutable ComPtr<ID2D1Bitmap> bitmap_;
};

class PainterImpl : public graph2d::PainterInterface
{
public:
    PainterImpl(ID2D1RenderTarget* rt, const scene2d::PointF& mouse_pos)
        : p_(rt, mouse_pos) {}
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
    void setRotation(float degrees, const scene2d::PointF& center, bool combine) override
    {
        if (combine)
            p_.Rotate(degrees, center);
        else
            p_.SetRotation(degrees, center);
    }
    void pushClipRect(const scene2d::PointF& origin, const scene2d::DimensionF& size) override
    {
        p_.PushClipRect(origin, size);
    }
    void popClipRect() override
    {
        p_.PopClipRect();
    }
    void clear(const style::Color& c) override
    {
        p_.Clear(c);
    }
    void drawBox(const scene2d::RectF& padding_rect,
        const style::EdgeOffsetF& border_width,
        const style::CornerRadiusF& border_radius,
        const style::Color& background_color,
        const style::Color& border_color,
        const graph2d::BitmapInterface* background_image) override
    {
        auto rect1 = scene2d::RectF::fromLTRB(
            padding_rect.left - border_width.left,
            padding_rect.top - border_width.top,
            padding_rect.right + border_width.right,
            padding_rect.bottom + border_width.bottom);
        float max_border_width = std::max(
            { border_width.left, border_width.top, border_width.right, border_width.bottom });
        p_.SetStrokeWidth(max_border_width);
        p_.SetStrokeColor(border_color);
        p_.SetColor(background_color);
        float max_border_raidus = std::max(
            { border_radius.top_left, border_radius.top_right, border_radius.bottom_right, border_radius.bottom_left });
        if (max_border_raidus > 0.0f) {
            p_.DrawRoundedRect(rect1.origin(), rect1.size(), max_border_raidus);
        } else {
            p_.DrawRect(rect1.origin(), rect1.size());
        }
        if (background_image) {
            auto bitmap = static_cast<const BitmapImpl*>(background_image)->d2dBitmap(p_);
            if (bitmap) {
                p_.DrawBitmap(bitmap, padding_rect.origin(), padding_rect.size());
            }
        }
    }
    void drawGlyphRun(const scene2d::PointF& pos, const graph2d::GlyphRunInterface* gr, const style::Color& color) override
    {
        p_.SetColor(color);
        auto glyph_run = (graphics::GlyphRun*)gr;
        p_.DrawGlyphRun(pos, *glyph_run);
    }
    void drawControl(const scene2d::RectF& rect, scene2d::Control* control) override
    {
        control->onPaint(*this, rect);
    }
private:
    graphics::Painter p_;
};

} // namespace graphics
} // namespace windows
