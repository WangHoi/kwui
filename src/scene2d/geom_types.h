#pragma once
#include <stdint.h>
#include <math.h>
#include "absl/strings/str_format.h"
#ifdef _WIN32
#include "windows/windows_header.h"
#endif

namespace scene2d {

struct PointF;
struct DimensionF;
struct RectF;

struct PointF {
    float x;
    float y;
    PointF(): x(0), y(0) {}
    PointF(float x_, float y_)
        : x(x_), y(y_) {}
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
    friend void AbslStringify(Sink& sink, const PointF& p) {
        absl::Format(&sink, "Point(%.2f, %.2f)", p.x, p.y);
    }
};

struct DimensionF {
    float width;
    float height;

    DimensionF()
        : width(0), height(0) {}
    DimensionF(float w, float h)
        : width(w), height(h) {}
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
    inline DimensionF& operator*=(const PointF &p)
    {
        width *= p.x;
        height *= p.y;
        return *this;
    }
    inline DimensionF operator*(const PointF &p) const
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
    inline DimensionF makeRound() const
    {
        return DimensionF(roundf(width), roundf(height));
    }

    template <typename Sink>
    friend void AbslStringify(Sink& sink, const DimensionF& p) {
        absl::Format(&sink, "Dim(%.2f, %.2f)", p.width, p.height);
    }
};

struct RectF {
    RectF()
        : left(0), top(0), right(0), bottom(0) {}
    static inline RectF fromOriginSize(const PointF& o, const DimensionF& s) {
        return RectF(o.x, o.y, o.x + s.width, o.y + s.height);
    }
    static inline RectF fromZeros() {
        return RectF(0.0f, 0.0f, 0.0f, 0.0f);
    }
    static inline RectF fromXYWH(float x, float y, float w, float h) {
        return RectF(x, y, x + w, y + h);
    }
    static inline RectF fromLTRB(float l, float t, float r, float b) {
        return RectF(l, t, r, b);
    }
    inline PointF origin() const {
        return PointF(left, top);
    }
    inline DimensionF size() const {
        return DimensionF(right - left, bottom - top);
    }
    inline PointF center() const {
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
        : left(l), top(t), right(r), bottom(b) {}

    template <typename Sink>
    friend void AbslStringify(Sink& sink, const RectF& p) {
        absl::Format(&sink, "RectF(%v,%v-%vx%v)",
            p.left, p.top, p.right - p.left, p.bottom - p.top);
    }
};

}
