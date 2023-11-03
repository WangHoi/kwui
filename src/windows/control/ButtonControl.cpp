#include "ButtonControl.h"
#include "ButtonControl.h"
#include "ButtonControl.h"
#include "windows/graphics/GraphicDevice.h"
#include "windows/graphics/Painter.h"
#include "scene2d/Scene.h"
#include "script/script.h"

namespace windows {
namespace control {

const char* ButtonControl::CONTROL_NAME = "button";

ButtonControl::ButtonControl()
{}

base::string_atom ButtonControl::name()
{
	return base::string_intern(CONTROL_NAME);
}

bool ButtonControl::testFlags(int flags) const
{
	return (scene2d::NODE_FLAG_CLICKABLE | scene2d::NODE_FLAG_HOVERABLE) & flags;
}

void ButtonControl::onPaint(graph2d::PainterInterface& p, const scene2d::RectF& rect)
{
	//p.SetColor(BLUE.MakeAlpha(0.2f));
	//p.DrawRect(rect.origin(), rect.size());
}

void ButtonControl::onMouseEvent(scene2d::Node* node, scene2d::MouseEvent& evt)
{
	node->requestPaint();
	if ((evt.cmd == scene2d::MOUSE_UP) && (evt.button & scene2d::LEFT_BUTTON) && (evt.buttons == 0)) {
		LOG(INFO) << "button click";
		JSContext* jctx = node->scene()->scriptContext().get();
		if (JS_IsFunction(jctx, onclick_func_)) {
			JS_EvalFunction(jctx, onclick_func_);
		}
	}
}

void ButtonControl::onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value)
{

}

void ButtonControl::onSetEventHandler(base::string_atom name, JSValue func)
{
	if (name == base::string_intern("onclick")) {
		onclick_func_ = func;
	}
}

}
}