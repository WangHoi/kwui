#pragma once
#include "api/kwui/ScriptValue.h"
#include "script.h"
#include <vector>

namespace script {

class EventPort {
public:
	static JSClassID JS_CLASS_ID;
	static const char* JS_CLASS_NAME;

	static bool postEvent(int port, const kwui::ScriptValue& val);

	static JSValue constructor(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
	static void finalizer(JSRuntime* rt, JSValue val);
	static void gcMark(JSRuntime* rt, JSValueConst val, JS_MarkFunc* mark_func);
	static JSValue addListener(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
	static JSValue removeListener(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

	static EventPort* findEventPort(int id);

	bool addListener(JSClassID func_class_id, kwui::ScriptFunction* func);
	bool removeListener(JSClassID func_class_id, kwui::ScriptFunction* func);
	
	inline JSContext* context() const
	{
		return ctx_;
	}
	inline int id() const
	{
		return id_;
	}

private:
	JSContext* ctx_ = nullptr;
	int id_ = 0;
	std::vector<Value> listeners_;
};

void js_add_event_port(JSContext* ctx);

}