#include "BitmapX.h"
#include "GraphicDeviceX.h"

namespace xskia {

scene2d::DimensionF BitmapFromUrlX::pixelSize() const
{
    if (!image_) {
        auto item = GraphicDeviceX::instance()->getBitmap(url_, 1.0f);
        if (item) {
            auto dim = item.frame->dimensions();
            return scene2d::DimensionF(dim.fWidth, dim.fHeight);
        } else {
            return scene2d::DimensionF();
        }
    }
    auto dim = image_->dimensions();
    return scene2d::DimensionF(dim.fWidth, dim.fHeight);
}
sk_sp<SkImage> BitmapFromUrlX::skImage() const
{
    if (!image_) {
        BitmapSubItemX item = GraphicDeviceX::instance()
            ->getBitmap(url_, 1.0f);
        if (item)
            image_ = item.frame;
    }

    return image_;
}

scene2d::DimensionF BitmapX::pixelSize() const
{
    if (!image_) {
        return scene2d::DimensionF();
    }
    auto dim = image_->dimensions();
    return scene2d::DimensionF(dim.fWidth, dim.fHeight);
}

}