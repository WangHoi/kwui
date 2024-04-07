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
    scene2d::DimensionF size() override;
    ID2D1Bitmap* d2dBitmap(Painter& p) const
    {
        if (!image_) {
            BitmapSubItem item = GraphicDevice::instance()
                ->getBitmap(url_, p.GetDpiScale());
            if (item)
                image_ = p.CreateBitmap(item);
        }

        return image_.Get();
    }
private:
    std::string url_; // utf-8
    mutable sk_sp<SkImage> image_;
};

}