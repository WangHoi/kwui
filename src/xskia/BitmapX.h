#pragma once
#include "graph2d/Bitmap.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"

namespace xskia {

class BitmapXInterface : public graph2d::BitmapInterface {
public:
    virtual ~BitmapXInterface() = default;
    virtual sk_sp<SkImage> skImage() const = 0;

private:
    std::string url_; // utf-8
    mutable sk_sp<SkImage> image_;
};

class BitmapFromUrlX : public BitmapXInterface {
public:
    BitmapFromUrlX(const std::string& url)
        : url_(url)
    {
    }
    const std::string& url() const
    {
        return url_;
    }
    scene2d::DimensionF pixelSize() const override;
    float dpiScale(float requested_dpi_scale) const override;
    sk_sp<SkImage> skImage() const override;

private:
    std::string url_; // utf-8
    mutable sk_sp<SkImage> image_;
};

class BitmapX : public BitmapXInterface {
public:
    BitmapX(sk_sp<SkImage> img, float dpi_scale = 1)
        : image_(img), dpi_scale_(dpi_scale)
    {
    }
    scene2d::DimensionF pixelSize() const override;
    float dpiScale(float /*requested_dpi_scale*/) const override
    {
        return dpi_scale_;
    }
    sk_sp<SkImage> skImage() const
    {
        return image_;
    }

private:
    mutable sk_sp<SkImage> image_;
    float dpi_scale_ = 1;
};

}