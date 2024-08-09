#include "ButtonControl.h"
#include "Scene.h"
#include "script/script.h"

namespace scene2d {

const char* ButtonControl::CONTROL_NAME = "button";

ButtonControl::ButtonControl()
{}

ButtonControl::~ButtonControl()
{
	int k = 1;
}

base::string_atom ButtonControl::name()
{
	return base::string_intern(CONTROL_NAME);
}

bool ButtonControl::hitTest(const scene2d::PointF& pos, int flags) const
{
	return (scene2d::NODE_FLAG_CLICKABLE | scene2d::NODE_FLAG_HOVERABLE) & flags;
}

void ButtonControl::onPaint(graph2d::PaintContextInterface& p, const scene2d::RectF& rect)
{
	//p.SetColor(BLUE.MakeAlpha(0.2f));
	//p.DrawRect(rect.origin(), rect.size());
}

void ButtonControl::onMouseEvent(scene2d::Node* node, scene2d::MouseEvent& evt)
{
	// LOG(INFO) << "ButtonControl::onMouseEvent isHandled=" << evt.isHandled()
	// 	<< ", cmd=" << evt.cmd
	// 	<< ", button=" << evt.button
	// 	<< ", buttons=" << evt.buttons;
	if (evt.isHandled())
		return;
	evt.setHandled();
	node->requestPaint();
	if ((evt.cmd == scene2d::MOUSE_UP) && (evt.button & scene2d::LEFT_BUTTON) && (evt.buttons == 0)) {
		// LOG(INFO) << "button click";
		JSContext* jctx = node->scene()->scriptContext().get();
		if (JS_IsFunction(jctx, onclick_func_.jsValue())) {
			auto func = onclick_func_;
			JSValue ret = JS_Call(jctx, func.jsValue(), JS_UNDEFINED, 0, nullptr);
			if (JS_IsException(ret)) {
				js_std_dump_error(jctx);
			}
			JS_FreeValue(jctx, ret);
		}
	}
}

void ButtonControl::onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value)
{

}

void ButtonControl::onSetEventHandler(base::string_atom name, const script::Value& func)
{
	if (name == base::string_intern("onclick")) {
		onclick_func_ = func;
	}
}

void ButtonControl::onDetach(scene2d::Node* node)
{
	onclick_func_ = script::Value();
}

}
