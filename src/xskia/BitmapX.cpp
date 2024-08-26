#include "BitmapX.h"
#include "GraphicDeviceX.h"

namespace xskia {

scene2d::DimensionF BitmapX::size() const
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
sk_sp<SkImage> BitmapX::skImage() const
{
    if (!image_) {
        BitmapSubItemX item = GraphicDeviceX::instance()
            ->getBitmap(url_, 1.0f);
        if (item)
            image_ = item.frame;
    }

    return image_;
}

}