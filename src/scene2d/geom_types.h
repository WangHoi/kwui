#pragma once
#include <stdint.h>
#include <math.h>
#include "absl/strings/str_format.h"
#ifdef _WIN32
#include "windows/windows_header.h"
#endif
#if WITH_SKIA
#include "include/core/SkRect.h"
#include "include/core/SkRRect.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSize.h"
#endif

namespace scene2d
{
struct PointF;
struct DimensionF;
struct RectF;

struct PointF
{
    float x;
    float y;

    PointF(): x(0), y(0)
    {
    }

    PointF(float x_, float y_)
        : x(x_), y(y_)
    {
    }

    static inline PointF fromZeros()
    {
        return PointF(0.0f, 0.0f);
    }

    static inline PointF fromAll(float f)
    {
        return PointF(f, f);
    }
#ifdef _WIN32
    inline operator D2D1_POINT_2F() const
    {
        return D2D1::Point2F(x, y);
    }
#endif
#if WITH_SKIA
    inline operator SkPoint() const
    {
        return SkPoint::Make(x, y);
    }
#endif
    inline PointF operator-() const
    {
        return PointF(-x, -y);
    }

    inline PointF& operator+=(float f)
    {
        x += f;
        y += f;
        return *this;
    }

    inline PointF operator+(float f) const
    {
        return PointF(x + f, y + f);
    }

    inline PointF& operator+=(const PointF& p)
    {
        x += p.x;
        y += p.y;
        return *this;
    }

    inline PointF operator+(const PointF& p) const
    {
        return PointF(x + p.x, y + p.y);
    }

    inline PointF& operator-=(float f)
    {
        x -= f;
        y -= f;
        return *this;
    }

    inline PointF operator-(float f) const
    {
        return PointF(x - f, y - f);
    }

    inline PointF& operator-=(const PointF& p)
    {
        x -= p.x;
        y -= p.y;
        return *this;
    }

    inline PointF operator-(const PointF& p) const
    {
        return PointF(x - p.x, y - p.y);
    }

    inline PointF& operator*=(float f)
    {
        x *= f;
        y *= f;
        return *this;
    }

    inline PointF operator*(float f) const
    {
        return PointF(x * f, y * f);
    }

    inline PointF& operator/=(float f)
    {
        x /= f;
        y /= f;
        return *this;
    }

    inline PointF operator/(float f) const
    {
        return PointF(x / f, y / f);
    }

    inline PointF makeRound() const
    {
        return PointF(roundf(x), roundf(y));
    }

    template <typename Sink>
    friend void AbslStringify(Sink& sink, const PointF& p)
    {
        absl::Format(&sink, "Point(%.2f, %.2f)", p.x, p.y);
    }
};

struct DimensionF
{
    float width;
    float height;

    DimensionF()
        : width(0), height(0)
    {
    }

    DimensionF(float w, float h)
        : width(w), height(h)
    {
    }

    static inline DimensionF fromZeros()
    {
        return DimensionF(0.0f, 0.0f);
    }

    static inline DimensionF fromAll(float f)
    {
        return DimensionF(f, f);
    }
#ifdef _WIN32
    inline operator D2D1_SIZE_F() const
    {
        return D2D1::SizeF(width, height);
    }
#endif
#if WITH_SKIA
    inline operator SkSize() const
    {
        return SkSize::Make(width, height);
    }
#endif
    inline DimensionF& operator*=(const PointF& p)
    {
        width *= p.x;
        height *= p.y;
        return *this;
    }

    inline DimensionF operator*(const PointF& p) const
    {
        return DimensionF(width * p.x, height * p.y);
    }

    inline DimensionF& operator*=(float f)
    {
        width *= f;
        height *= f;
        return *this;
    }

    inline DimensionF operator*(float f) const
    {
        return DimensionF(width * f, height * f);
    }

    inline DimensionF& operator/=(const PointF& p)
    {
        width /= p.x;
        height /= p.y;
        return *this;
    }

    inline DimensionF operator/(const PointF& p) const
    {
        return DimensionF(width / p.x, height / p.y);
    }

    inline DimensionF& operator/=(float f)
    {
        width /= f;
        height /= f;
        return *this;
    }

    inline DimensionF operator/(float f) const
    {
        return DimensionF(width / f, height / f);
    }

    bool isZeros() const
    {
        return width == 0 && height == 0;
    }

    inline DimensionF makeRound() const
    {
        return DimensionF(roundf(width), roundf(height));
    }

    template <typename Sink>
    friend void AbslStringify(Sink& sink, const DimensionF& p)
    {
        absl::Format(&sink, "Dim(%.2f, %.2f)", p.width, p.height);
    }
};

struct RectF
{
    RectF()
        : left(0), top(0), right(0), bottom(0)
    {
    }

    static inline RectF fromOriginSize(const PointF& o, const DimensionF& s)
    {
        return RectF(o.x, o.y, o.x + s.width, o.y + s.height);
    }

    static inline RectF fromZeros()
    {
        return RectF(0.0f, 0.0f, 0.0f, 0.0f);
    }

    static inline RectF fromXYWH(float x, float y, float w, float h)
    {
        return RectF(x, y, x + w, y + h);
    }

    static inline RectF fromLTRB(float l, float t, float r, float b)
    {
        return RectF(l, t, r, b);
    }
#ifdef _WIN32
    inline operator D2D1_RECT_F() const
    {
        return D2D1::RectF(left, top, right, bottom);
    }
#endif
#if WITH_SKIA
    inline operator SkRect() const
    {
        return SkRect::MakeLTRB(left, top, right, bottom);
    }
#endif
    inline PointF origin() const
    {
        return PointF(left, top);
    }

    inline DimensionF size() const
    {
        return DimensionF(right - left, bottom - top);
    }

    inline PointF center() const
    {
        return PointF((left + right) * 0.5f, (top + bottom) * 0.5f);
    }

    inline float width() const { return right - left; }
    inline float height() const { return bottom - top; }

    bool contains(const PointF& pos) const
    {
        return (left <= pos.x && pos.x < right && top <= pos.y && pos.y < bottom);
    }

    RectF& translate(const PointF& pos)
    {
        left += pos.x;
        right += pos.x;
        top += pos.y;
        bottom += pos.y;
        return *this;
    }

    RectF translated(const PointF& pos) const
    {
        return RectF(left + pos.x, top + pos.y, right + pos.x, bottom + pos.y);
    }

    RectF& unite(const RectF& r)
    {
        left = std::min(left, r.left);
        right = std::max(right, r.right);
        top = std::min(top, r.top);
        bottom = std::max(bottom, r.bottom);
        return *this;
    }

    RectF united(const RectF& r) const
    {
        return RectF(std::min(left, r.left),
                     std::min(top, r.top),
                     std::max(right, r.right),
                     std::max(bottom, r.bottom));
    }

    RectF& intersect(const RectF& r)
    {
        left = std::min(std::max(left, r.left), right);
        top = std::min(std::max(top, r.top), bottom);
        right = std::max(std::min(right, r.right), left);
        bottom = std::max(std::min(bottom, r.bottom), top);
        return *this;
    }

    RectF intersected(const RectF& r) const
    {
        return RectF(std::min(std::max(left, r.left), right)/* left */,
                     std::min(std::max(top, r.top), bottom)/* top */,
                     std::max(std::min(right, r.right), left)/* right */,
                     std::max(std::min(bottom, r.bottom), top) /* bottom */);
    }

    RectF& moveTo(float x, float y)
    {
        right = x + (right - left);
        bottom = y + (bottom - top);
        left = x;
        top = y;
        return *this;
    }

    inline RectF& moveTo(const PointF& p)
    {
        return moveTo(p.x, p.y);
    }

    RectF& adjust(float dl, float dt, float dr, float db)
    {
        left += dl;
        right += dr;
        top += dt;
        bottom += db;
        return *this;
    }

    RectF adjusted(float dl, float dt, float dr, float db) const
    {
        return RectF(left + dl, top + dt, right + dr, bottom + db);
    }

    float left;
    float top;
    float right;
    float bottom;

private:
    RectF(float l, float t, float r, float b)
        : left(l), top(t), right(r), bottom(b)
    {
    }

    template <typename Sink>
    friend void AbslStringify(Sink& sink, const RectF& p)
    {
        absl::Format(&sink, "RectF(%v,%v-%vx%v)",
                     p.left, p.top, p.right - p.left, p.bottom - p.top);
    }
};


struct CornerRadiusF
{
    DimensionF top_left;
    DimensionF top_right;
    DimensionF bottom_right;
    DimensionF bottom_left;

    CornerRadiusF() = default;

    explicit CornerRadiusF(float f)
        : top_left(f, f), top_right(f, f), bottom_right(f, f), bottom_left(f, f)
    {
    }

    template <typename Sink>
    friend void AbslStringify(Sink& sink, const CornerRadiusF& o)
    {
        absl::Format(&sink, "CornerRadiusF { ");
        if (o.top_left.x != 0.0f || o.top_left.y != 0.0f)
            absl::Format(&sink, "top_left=%v, ", o.top_left);
        if (o.top_right.x != 0.0f || o.top_right.y != 0.0f)
            absl::Format(&sink, "top_right=%v, ", o.top_right);
        if (o.bottom_right.x != 0.0f || o.bottom_right.y != 0.0f)
            absl::Format(&sink, "bottom_right=%v, ", o.bottom_right);
        if (o.bottom_left.x != 0.0f || o.bottom_left.y != 0.0f)
            absl::Format(&sink, "bottom_left=%v, ", o.bottom_left);
        absl::Format(&sink, "}");
    }

    bool isSymmetric() const
    {
        return (top_left.width == top_right.width
            && top_left.width == bottom_left.width
            && bottom_left.width == bottom_right.width
            && top_left.height == bottom_left.height
            && top_right.height == bottom_right.height
            && top_left.height == top_right.height);
    }
};

struct RRectF
{
    static inline RRectF fromRectRadius(const RectF& rect, const CornerRadiusF& radius)
    {
        return RRectF{rect, radius};
    }

    static inline RRectF fromZeros()
    {
        return RRectF(RectF{}, CornerRadiusF{});
    }

#if WITH_SKIA
    inline operator SkRRect() const
    {
        SkRRect rr;
        SkVector radii[4] = {
            {corner_radius.top_left.width, corner_radius.top_left.height},
            {corner_radius.top_right.width, corner_radius.top_right.height},
            {corner_radius.bottom_right.width, corner_radius.bottom_right.height},
            {corner_radius.bottom_left.width, corner_radius.bottom_left.height},
        };
        rr.setRectRadii(rect, radii);
        return rr;
    }
#endif

    inline PointF origin() const
    {
        return rect.origin();
    }

    inline DimensionF size() const
    {
        return rect.size();
    }

    inline PointF center() const { return rect.center(); }
    inline float width() const { return rect.width(); }
    inline float height() const { return rect.height(); }

    bool isRect() const
    {
        return !(std::min(corner_radius.top_left.width, corner_radius.top_left.height) > 0
            || std::min(corner_radius.top_right.width, corner_radius.top_right.height) > 0
            || std::min(corner_radius.bottom_right.width, corner_radius.bottom_right.height) > 0
            || std::min(corner_radius.bottom_left.width, corner_radius.bottom_left.height) > 0);
    }

    bool contains(const PointF& pos) const
    {
        return rect.contains(pos);
    }

    RRectF& translate(const PointF& pos)
    {
        rect.translate(pos);
        return *this;
    }

    RRectF translated(const PointF& pos) const
    {
        return RRectF(rect.translated(pos), corner_radius);
    }

    RRectF& moveTo(float x, float y)
    {
        rect.moveTo(x, y);
        return *this;
    }

    inline RRectF& moveTo(const PointF& p)
    {
        return moveTo(p.x, p.y);
    }

    RRectF& adjust(float dl, float dt, float dr, float db)
    {
        rect.adjust(dl, dt, dr, db);
        return *this;
    }

    RRectF adjusted(float dl, float dt, float dr, float db) const
    {
        return RRectF(rect.adjusted(dl, dt, dr, db), corner_radius);
    }

    RectF rect;
    CornerRadiusF corner_radius;

private:
    RRectF(const RectF& rc, const CornerRadiusF& radius)
        : rect(rc), corner_radius(radius)
    {
    }

    template <typename Sink>
    friend void AbslStringify(Sink& sink, const RRectF& p)
    {
        absl::Format(&sink, "RRectF(%v, %v)", p.rect, p.corner_radius);
    }
};
}
