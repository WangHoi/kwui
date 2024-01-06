#pragma once
#include "quickjs.h"
#include <vector>

namespace scene2d {
class Node;
}

JSValue component_state_render(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

namespace script {
class ComponentState {
public:
	static JSClassID JS_CLASS_ID;
	
	// register class to global object
	static void registerClass(JSContext* ctx);
	static JSValue newObject(JSContext* ctx, int argc, JSValueConst* argv);
	static void setNode(JSContext* ctx, JSValue this_val, scene2d::Node* node);

	JSValue useHook(JSContext* ctx, JSValue this_val, JSValue init_fn, JSValue update_fn, JSValue cleanup_fn);
	void finalize(JSRuntime* rt);

	static void gcMark(JSRuntime* rt, JSValueConst val, JS_MarkFunc* mark_func);
	static JSValue dialogId(JSContext* ctx, JSValueConst this_val);

	ComponentState(JSValue this_obj)
		: this_obj_(this_obj) {}

private:
	static JSValue useHookUpdater(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic, JSValue* func_data);
	void unmount(JSContext* ctx, JSValueConst this_val);

	struct Slot {
		JSValue state = JS_UNINITIALIZED;
		JSValue updateFn = JS_UNINITIALIZED;
		JSValue cleanupFn = JS_UNINITIALIZED;
	};

	JSValue this_obj_ = JS_UNDEFINED;
	scene2d::Node* node_ = nullptr;
	size_t curr_slot_ = 0;
	std::vector<Slot> slots_;

	friend class scene2d::Node;
	friend JSValue component_state_render(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
};

}