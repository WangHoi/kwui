#pragma once
#include "base/WeakObject.h"
#include "quickjs.h"
#include "Value.h"
#include "base/TaskQueue.h"
#include <vector>
#include <map>
#include <functional>

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
	JSValue provideContext(JSContext* ctx, JSValue this_val, JSValue id, JSValue user_ctx);
	JSValue useContext(JSContext* ctx, JSValue this_val, JSValue id);
	JSValue useEffect(JSContext* ctx, JSValue this_val, JSValue effect_fn, JSValue deps, JSValue compare_fn);
	void finalize(JSRuntime* rt);

	static void gcMark(JSRuntime* rt, JSValueConst val, JS_MarkFunc* mark_func);
	static JSValue dialogId(JSContext* ctx, JSValueConst this_val);

	ComponentState(JSValue this_obj);
	~ComponentState();

private:
	static JSValue useHookUpdater(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic, JSValue* func_data);
	void unmount(JSContext* ctx, JSValueConst this_val);
	ComponentState* parent() const;
	inline base::object_weakptr<ComponentState> weaken() const
	{
		return base::object_weakptr<ComponentState>(weak_proxy_);
	}
	std::function<void()> makeUseEffectTask(JSContext* ctx, size_t index) const;
	void runEffect(JSContext* ctx, size_t index);

	struct Slot {
		JSValue state = JS_UNINITIALIZED;
		JSValue update_fn = JS_UNINITIALIZED;
		JSValue cleanup_fn = JS_UNINITIALIZED;
	};
	struct Effect {
		enum State {
			STATE_INIT,
			STATE_SCHEDULED,
			STATE_EXECUTED,
		} state = STATE_INIT;
		base::TaskQueue::TaskId tid = 0;
		Value deps;
		Value setup_fn;
		Value cleanup_fn;
	};

	base::WeakObjectProxy<ComponentState>* weak_proxy_ = nullptr;
	JSValue this_obj_ = JS_UNDEFINED;
	scene2d::Node* node_ = nullptr;
	size_t curr_slot_ = 0;
	std::vector<Slot> slots_;
	std::map<size_t, Value> contexts_;
	size_t curr_effect_ = 0;
	std::vector<Effect> effects_;

	friend class scene2d::Node;
	friend JSValue component_state_render(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
};

}