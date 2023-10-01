#pragma once
#include <stdint.h>
#include <math.h>

namespace scene2d {

struct PointF;
struct Point;
struct DimensionF;
struct Dimension;
struct RectF;
struct Rect;

struct PointF {
    float x;
    float y;
    PointF() {} // Uninitialized
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
};

struct Point {
    int32_t x;
    int32_t y;
};

struct DimensionF {
    float width;
    float height;

    DimensionF() {} // Uninitialized
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
};

struct Dimension {
    int32_t width;
    int32_t height;
};

struct RectF {
    RectF() {} // Uninitialized
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
    float left;
    float top;
    float right;
    float bottom;
private:
    RectF(float l, float t, float r, float b)
        : left(l), top(t), right(r), bottom(b) {}
};

struct Rect {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
};

}