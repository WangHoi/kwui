#include "BitmapX.h"
#include "GraphicDeviceX.h"

namespace xskia {

scene2d::DimensionF BitmapX::size()
{
    if (!image_) {
        auto item = GraphicDeviceX::instance()->getBitmap(url_, 1.0f);
        if (item) {
            UINT w, h;
            item.frame->GetSize(&w, &h);
            return scene2d::DimensionF((float)w, (float)h);
        } else {
            return scene2d::DimensionF();
        }
    }
    auto dim = image_->dimensions();
    return scene2d::DimensionF(dim.fWidth, dim.fHeight);
}

}