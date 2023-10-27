#include "ImageControl.h"
#include "windows/graphics/GraphicDevice.h"
#include "windows/graphics/Painter.h"

namespace windows {
namespace control {

const char* ImageControl::CONTROL_NAME = "img";

ImageControl::ImageControl()
{}

base::string_atom ImageControl::name()
{
	return base::string_intern(CONTROL_NAME);
}
void ImageControl::onPaint(graph2d::PainterInterface& pi, const scene2d::RectF& rect)
{
	graphics::Painter& p = graphics::PainterImpl::unwrap(pi);
	if (!_bitmap) {
		graphics::BitmapSubItem item = graphics::GraphicDevice::instance()
			->GetBitmap(_image_src, p.GetDpiScale());
		if (item)
			_bitmap = p.CreateBitmap(item);
	}
	if (_bitmap) {
		p.DrawBitmap(_bitmap.Get(), rect.origin(), rect.size());
	}
}
void ImageControl::onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value)
{
	if (name == base::string_intern("src")) {
		setImageSource(absl::get<std::string>(value));
	}
}
void ImageControl::setImageSource(const std::string& src) {
	_image_src = src;
}

}
}
