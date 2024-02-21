#include "ElementRef.h"
#include "scene2d/Scene.h"
#include "windows/Dialog.h"

namespace script {

JSClassID ElementRef::JS_CLASS_ID = 0;
JSClassDef ElementRef::JS_CLASS_DEF = {
	"__ElementRef",
	&ElementRef::finalize,
};
const JSCFunctionListEntry ElementRef::JS_FUNCTION_LIST[1] = {
	js_cfunc_def("getScreenRect", 0, &ElementRef::js_getScreenRect),
};

void ElementRef::registerClass(JSContext* ctx)
{
	JSRuntime* rt = JS_GetRuntime(ctx);
	JS_NewClassID(&JS_CLASS_ID);
	JSValue proto = JS_NewObject(ctx);
	JS_SetPropertyFunctionList(ctx, proto, JS_FUNCTION_LIST, ABSL_ARRAYSIZE(JS_FUNCTION_LIST));
	JS_NewClass(rt, JS_CLASS_ID, &JS_CLASS_DEF);
	JS_SetClassProto(ctx, JS_CLASS_ID, proto);
}

JSValue ElementRef::create(JSContext* ctx, const base::object_weakptr<scene2d::Node>& node)
{
	JSValue val = JS_NewObjectClass(ctx, JS_CLASS_ID);
	JS_SetOpaque(val, new ElementRef(node));
	return val;
}

ElementRef::ElementRef(const base::object_weakptr<scene2d::Node>& node)
	: node_link_(node)
{
}

void ElementRef::finalize(JSRuntime* rt, JSValue val)
{
	auto me = (ElementRef*)JS_GetOpaque(val, JS_CLASS_ID);
	delete me;
}

JSValue ElementRef::js_getScreenRect(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
	auto me = (ElementRef*)JS_GetOpaque(this_val, JS_CLASS_ID);
	auto node = me->node_link_.upgrade();
	if (!node || !node->scene())
		return JS_UNDEFINED;

	auto o = &node->layoutObject();
	auto rect = style::LayoutObject::borderRect(o);
	auto offset = rect.origin() - style::LayoutObject::contentRect(o).origin();
	auto scene_pos = node->scene()->mapPointToScene(node.get(), offset);
	rect = scene2d::RectF::fromOriginSize(scene_pos, rect.size());
	
	// convert to window coords
	auto dlg = windows::Dialog::findDialogById(node->scene()->eventContextId());
	if (dlg) {
		float dpi_scale = dlg->GetDpiScale();
		POINT top_left = { lroundf(rect.left * dpi_scale), lroundf(rect.top * dpi_scale) };
		POINT bottom_right = { lroundf(rect.right * dpi_scale), lroundf(rect.bottom * dpi_scale) };
		::ClientToScreen(dlg->GetHwnd(), &top_left);
		::ClientToScreen(dlg->GetHwnd(), &bottom_right);
		
		rect = scene2d::RectF::fromLTRB(top_left.x, top_left.y, bottom_right.x, bottom_right.y);
	} else {
		LOG(ERROR) << "elementRef::getScreenRect(): dialog not found";
	}

	JSValue j = JS_NewObject(ctx);
	JS_SetPropertyStr(ctx, j, "left", JS_NewFloat64(ctx, rect.left));
	JS_SetPropertyStr(ctx, j, "top", JS_NewFloat64(ctx, rect.top));
	JS_SetPropertyStr(ctx, j, "right", JS_NewFloat64(ctx, rect.right));
	JS_SetPropertyStr(ctx, j, "bottom", JS_NewFloat64(ctx, rect.bottom));
	return j;
}

}