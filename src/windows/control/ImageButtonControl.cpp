#include "ImageButtonControl.h"
#include "windows/graphics/GraphicDevice.h"
#include "windows/graphics/PainterD2D.h"
#include "scene2d/Scene.h"
#include "script/script.h"

namespace windows {
namespace control {

const char* ImageButtonControl::CONTROL_NAME = "image_button";

ImageButtonControl::ImageButtonControl()
{}

base::string_atom ImageButtonControl::name()
{
	return base::string_intern(CONTROL_NAME);
}
bool ImageButtonControl::hitTest(const scene2d::PointF& pos, int flags) const
{
	return (scene2d::NODE_FLAG_CLICKABLE | scene2d::NODE_FLAG_HOVERABLE) & flags;
}
void ImageButtonControl::onPaint(graph2d::PainterInterface& pi, const scene2d::RectF& rect)
{
	graphics::Painter& p = graphics::PainterImpl::unwrap(pi);
	if (!_bitmap) {
		graphics::BitmapSubItem item = graphics::GraphicDevice::instance()
			->getBitmap(_src, p.GetDpiScale());
		if (item)
			_bitmap = p.CreateBitmap(item);
	}
	if (!_hover_bitmap) {
		graphics::BitmapSubItem item = graphics::GraphicDevice::instance()
			->getBitmap(_hover_src, p.GetDpiScale());
		if (item)
			_hover_bitmap = p.CreateBitmap(item);
	}
	if (_bitmap) {
		p.DrawBitmap(_bitmap.Get(), rect.origin(), rect.size());
	}
}
void ImageButtonControl::onMouseEvent(scene2d::Node* node, scene2d::MouseEvent& evt)
{
	node->requestPaint();
	if ((evt.cmd == scene2d::MOUSE_UP) && (evt.button & scene2d::LEFT_BUTTON) && (evt.buttons == 0)) {
		JSContext* jctx = node->scene()->scriptContext().get();
		if (JS_IsFunction(jctx, onclick_func_.jsValue())) {
			JS_EvalFunction(jctx, onclick_func_.jsValue());
		}
	}
}
void ImageButtonControl::onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value)
{
	if (name == base::string_intern("src")) {
		_src = value.toString();
		_bitmap = nullptr;
	} else if (name == base::string_intern("hover_src")) {
		_hover_src = value.toString();
		_hover_bitmap = nullptr;
	}
}

void ImageButtonControl::onSetEventHandler(base::string_atom name, const script::Value& func)
{
	if (name == base::string_intern("onclick")) {
		onclick_func_ = func;
	}
}


}
}
