#include "ImageControl.h"
#include "graph2d/graph2d.h"

namespace scene2d {

const char* ImageControl::CONTROL_NAME = "img";

ImageControl::ImageControl()
{}

base::string_atom ImageControl::name()
{
	return base::string_intern(CONTROL_NAME);
}
void ImageControl::onPaint(graph2d::PainterInterface& pi, const scene2d::RectF& rect)
{
	// TODO: bitmap DPI support
	if (!bitmap_) {
		bitmap_ = graph2d::createBitmap(image_src_);
	}
	if (bitmap_) {
		pi.drawBitmap(bitmap_.get(), rect.origin(), rect.size());
	}
}
void ImageControl::onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value)
{
	if (name == base::string_intern("src")) {
		setImageSource(value.toString());
	}
}
void ImageControl::setImageSource(const std::string& src) {
	image_src_ = src;
	bitmap_ = nullptr;
}

}
