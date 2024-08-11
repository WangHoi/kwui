#pragma once

#include "scene2d/geom_types.h"
#include "scene2d/Control.h"
#include "graph2d/PaintContextInterface.h"
#include "graph2d/Bitmap.h"
#include "windows/windows_header.h"
#include "TextLayoutD2D.h"
#include "TextFlowD2D.h"
#include "GraphicDeviceD2D.h"
#include <vector>

#include "graph2d/PaintPathInterface.h"

namespace windows::graphics
{
struct NativeBitmap
{
    float width = 0;
    float height = 0;
    float dpi_scale = 1;
    ComPtr<ID3D11Texture2D> d3d_tex;
    ComPtr<ID2D1Bitmap1> d2d_bitmap;

    operator bool() const
    {
        return d3d_tex != nullptr && d2d_bitmap != nullptr;
    }
};

class PainterImpl;

class Painter
{
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

    void DrawRect(const scene2d::PointF& origin, const scene2d::DimensionF& size)
    {
        DrawRect(origin.x, origin.y, size.width, size.height);
    }

    void DrawRoundedRect(float x, float y, float w, float h, float r);

    void DrawRoundedRect(const scene2d::PointF& origin, const scene2d::DimensionF& size, float r)
    {
        DrawRoundedRect(origin.x, origin.y, size.width, size.height, r);
    }

    void DrawCircle(float center_x, float center_y, float radius);
    void DrawCircle(const scene2d::PointF& center, float radius) { DrawCircle(center.x, center.y, radius); }
    void DrawArc(float center_x, float center_y, float radius, float start_angle, float span_angle);

    void DrawArc(const scene2d::PointF& center, float radius, float start_angle, float span_angle)
    {
        DrawArc(center.x, center.y, radius, start_angle, span_angle);
    }

    void DrawTextLayout(const scene2d::PointF& origin, const TextLayoutD2D& layout)
    {
        DrawTextLayout(origin.x, origin.y, layout);
    }

    void DrawTextLayout(float x, float y, const TextLayoutD2D& layout);
    void DrawGlyphRun(const scene2d::PointF& origin, const GlyphRun& gr) { DrawGlyphRun(origin.x, origin.y, gr); }
    void DrawGlyphRun(float x, float y, const GlyphRun& gr);
    void Translate(float x, float y) { Translate({x, y}); }
    void Translate(const scene2d::PointF& offset);
    void SetTranslation(const scene2d::PointF& abs_offset);
    void Rotate(float degrees, const scene2d::PointF& center);
    void SetRotation(float degrees, const scene2d::PointF& center);
    //const scene2d::PointF& GetAccumTranslate() const { return _current.offset; }
    void ClipRect(const scene2d::PointF& origin, const scene2d::DimensionF& size);
    void Save();
    void Restore();
    ComPtr<ID2D1Bitmap> CreateBitmap(const BitmapSubItem& item);

    inline void DrawBitmap(ID2D1Bitmap* bitmap, const scene2d::PointF& origin, const scene2d::DimensionF& size)
    {
        DrawBitmap(bitmap, origin.x, origin.y, size.width, size.height);
    }

    void DrawBitmap(ID2D1Bitmap* bitmap, float x, float y, float w, float h);
    void DrawScale9Bitmap(ID2D1Bitmap* bitmap, float x, float y, float w, float h,
                          float src_margin, float dst_margin);
    ComPtr<ID2D1LinearGradientBrush> CreateLinearGradientBrush_Logo();
    ComPtr<ID2D1RadialGradientBrush> CreateRadialGradientBrush_Highlight();
    ComPtr<ID2D1BitmapBrush> CreateBitmapBrush(ID2D1Bitmap* bitmap);
    void SetBrush(ComPtr<ID2D1Brush> brush);

    NativeBitmap createNativeBitmap(float width, float height);

private:
    ComPtr<ID2D1Brush> CreateBrush(const style::Color& c);

    inline float Painter::PixelSnap(float x)
    {
        return roundf(x * _dpi_scale) / _dpi_scale;
    }

    inline D2D1_POINT_2F Painter::PixelSnap(const D2D1_POINT_2F& p)
    {
        return D2D1::Point2F(PixelSnap(p.x), PixelSnap(p.y));
    }

    D2D1_RECT_F PixelSnap(const D2D1_RECT_F& rect);
    D2D1_RECT_F PixelSnapConservative(const D2D1_RECT_F& rect);

    struct State
    {
        //scene2d::PointF offset;
        D2D1::Matrix3x2F transform;
        style::Color color;
        style::Color stroke_color;
        float stroke_width;
        bool pixel_snap;
        ComPtr<ID2D1Brush> gradient_brush;
        int push_clip_rect_count;

        State()
        {
            Reset();
        }

        inline void Reset()
        {
            //offset = scene2d::PointF::fromZeros();
            transform = D2D1::Matrix3x2F::Identity();
            color = style::Color();
            stroke_color = style::Color();
            stroke_width = 0.0f;
            pixel_snap = true;
            gradient_brush = nullptr;
            push_clip_rect_count = 0;
        }

        inline bool HasFill() const
        {
            return color.getAlpha() != 0 || gradient_brush != nullptr;
        }

        inline bool HasStroke() const
        {
            return stroke_width != 0 && stroke_color.getAlpha() != 0;
        }
    };

    ID2D1RenderTarget* _rt;
    scene2d::PointF _mouse_position;
    float _dpi_scale;
    State _current;
    std::vector<State> _state_stack;

    friend class PainterImpl;
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
            BitmapSubItem item = GraphicDeviceD2D::instance()
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
            BitmapSubItem item = GraphicDeviceD2D::instance()
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

class PaintPathD2D : public graph2d::PaintPathInterface
{
public:
    void moveTo(float x, float y) override;
    void lineTo(float x, float y) override;
    void arcTo(float radius_x, float radius_y, float rotation_degress,
               graph2d::SweepDirection sweep_dir, graph2d::ArcSize arc_size,
               float x, float y) override;
    void close() override;
    void addRRect(const scene2d::RRectF& r);

    ABSL_MUST_USE_RESULT ID2D1PathGeometry* getD2D1PathGeometry() const
    {
        const_cast<PaintPathD2D*>(this)->finalize();
        return geometry_.Get();
    }

private:
    void updateCheck();
    void reset();
    void finalize();

    bool finalized_ = false;
    ComPtr<ID2D1PathGeometry> geometry_;
    ComPtr<ID2D1GeometrySink> gsink_;
    absl::optional<scene2d::PointF> firstp_, lastp_;
};

class PainterImpl : public graph2d::PaintContextInterface
{
public:
    PainterImpl(ID2D1RenderTarget* rt, const scene2d::PointF& mouse_pos)
        : p_(rt, mouse_pos)
    {
    }

    static inline graphics::Painter& unwrap(graph2d::PaintContextInterface& pi)
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

    float getDpiScale() override
    {
        return p_.GetDpiScale();
    }

    void translate(const scene2d::PointF& offset) override
    {
        p_.Translate(offset);
    }

    // void rotate(float radians, const scene2d::PointF& center) override
    // {
    //     p_.Rotate(radians, center);
    // }

    void clipRect(const scene2d::RectF& rect) override
    {
        p_.ClipRect(rect.origin(), rect.size());
    }

    void clear(const style::Color& c) override
    {
        p_.Clear(c);
    }

    void drawBox(const scene2d::RectF& padding_rect,
                 const style::EdgeOffsetF& border_width,
                 const scene2d::CornerRadiusF& border_radius,
                 const style::Color& background_color,
                 const style::Color& border_color,
                 const graph2d::BitmapInterface* background_image) override;

    void drawGlyphRun(const scene2d::PointF& pos, const style::GlyphRunInterface* gr,
                      const style::Color& color) override
    {
        p_.SetColor(color);
        auto glyph_run = (graphics::GlyphRun*)gr;
        p_.DrawGlyphRun(pos, *glyph_run);
    }

    void drawControl(const scene2d::RectF& rect, scene2d::Control* control) override
    {
        control->onPaint(*this, rect);
    }

    void drawBitmap(const graph2d::BitmapInterface* image,
                    const scene2d::PointF& origin,
                    const scene2d::DimensionF& size) override
    {
        auto bitmap = static_cast<const BitmapImpl*>(image)->d2dBitmap(p_);
        if (bitmap) {
            p_.DrawBitmap(bitmap, origin, size);
        }
    }

    void drawRoundedRect(const scene2d::RectF& rect,
                         const scene2d::CornerRadiusF& border_radius,
                         const style::Color& background_color) override
    {
        float r = std::max({
            border_radius.top_left.width,
            border_radius.top_left.height,
            border_radius.top_right.width,
            border_radius.top_right.height,
            border_radius.bottom_right.width,
            border_radius.bottom_right.height,
            border_radius.bottom_left.width,
            border_radius.bottom_left.height,
        });
        p_.SetColor(background_color);
        p_.DrawRoundedRect(rect.origin(), rect.size(), r);
    }

    void drawRect(const scene2d::RectF& rect,
                  const style::Color& background_color) override
    {
        p_.SetColor(background_color);
        p_.DrawRect(rect.origin(), rect.size());
    }

    void drawTextLayout(const scene2d::PointF& origin,
                        const scene2d::TextLayoutInterface& layout,
                        const style::Color& color) override
    {
        p_.SetColor(color);
        p_.DrawTextLayout(origin, (const windows::graphics::TextLayoutD2D&)layout);
    }

    virtual void drawCircle(const scene2d::PointF& center,
                            float radius,
                            const style::Color& background_color,
                            float border_width,
                            const style::Color& border_color) override
    {
        p_.SetStrokeWidth(border_width);
        p_.SetStrokeColor(border_color);
        p_.SetColor(background_color);
        p_.DrawCircle(center, radius);
    }

    virtual void drawArc(const scene2d::PointF& center,
                         float radius,
                         float start_angle,
                         float span_angle,
                         const style::Color& background_color,
                         float border_width,
                         const style::Color& border_color) override
    {
        p_.SetStrokeWidth(border_width);
        p_.SetStrokeColor(border_color);
        p_.SetColor(background_color);
        p_.DrawArc(center, radius, start_angle, span_angle);
    }

    // void scale(const scene2d::PointF& s) override;
    void drawRect(const scene2d::RectF& rect, const graph2d::PaintBrush& brush) override;
    void drawRRect(const scene2d::RRectF& rrect, const graph2d::PaintBrush& brush) override;
    void drawDRRect(const scene2d::RRectF& outer, const scene2d::RRectF& inner,
                    const graph2d::PaintBrush& brush) override;
    void drawPath(const graph2d::PaintPathInterface* path, const graph2d::PaintBrush& brush) override;

private:
    graphics::Painter p_;
};
} // namespace windows::graphics
