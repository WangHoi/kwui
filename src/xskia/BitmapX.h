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
    sk_sp<SkImage> skImage() const;

private:
    std::string url_; // utf-8
    mutable sk_sp<SkImage> image_;
};

class BitmapX : public BitmapXInterface {
public:
    BitmapX(sk_sp<SkImage> img)
        : image_(img)
    {
    }
    scene2d::DimensionF pixelSize() const override;
    sk_sp<SkImage> skImage() const
    {
        return image_;
    }

private:
    std::string url_; // utf-8
    mutable sk_sp<SkImage> image_;
};

}