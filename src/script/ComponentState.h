#pragma once
#include "quickjs.h"
#include <vector>

namespace scene2d {
class Node;
}

namespace script {
class ComponentState {
public:
	static JSClassID JS_CLASS_ID;
	
	// register class to global object
	static void registerClass(JSContext* ctx);
	static JSValue newObject(JSContext* ctx, int argc, JSValueConst* argv);
	static void setNode(JSValue this_val, scene2d::Node* node);

	JSValue useHook(JSContext* ctx, JSValue this_val, JSValue initial_state, JSValue mutater);

private:
	static JSValue useHookUpdater(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic, JSValue* func_data);

	struct Slot {
		JSValue state = JS_UNINITIALIZED;
		JSValue mutater = JS_UNINITIALIZED;
	};

	scene2d::Node* node_ = nullptr;
	size_t curr_slot_ = 0;
	std::vector<Slot> slots_;

	friend class scene2d::Node;
};

}