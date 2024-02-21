#pragma once
#include "quickjs.h"
#include "scene2d/Node.h"

namespace script {

class ElementRef {
public:
	static JSClassID JS_CLASS_ID;
	static JSClassDef JS_CLASS_DEF;
	static const JSCFunctionListEntry JS_FUNCTION_LIST[];
	static void registerClass(JSContext* ctx);

	static JSValue create(JSContext* ctx, const base::object_weakptr<scene2d::Node>& node);

	ElementRef(const base::object_weakptr<scene2d::Node>& node);
	~ElementRef() = default;

private:
	static void finalize(JSRuntime* rt, JSValue val);
	static JSValue js_getScreenRect(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

	base::object_weakptr<scene2d::Node> node_link_;
};

}