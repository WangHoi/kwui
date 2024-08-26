#pragma once
#include "graph2d/Bitmap.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"

namespace xskia {

class BitmapX : public graph2d::BitmapInterface {
public:
    BitmapX(const std::string& url)
        : url_(url)
    {
    }
    const std::string& url() const override
    {
        return url_;
    }
    scene2d::DimensionF size() const override;
    sk_sp<SkImage> skImage() const;

private:
    std::string url_; // utf-8
    mutable sk_sp<SkImage> image_;
};

}