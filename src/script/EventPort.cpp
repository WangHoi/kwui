#include "EventPort.h"
#include "absl/base/macros.h"
#include <unordered_map>

namespace script {

struct NativeSubItem {
	kwui::ScriptFunction* func;
	void* udata;
};

struct ScriptSubItem {
	JSContext* ctx;
	JSValue func;
};

struct Subscription {
	std::vector<NativeSubItem> native_subs;
	std::vector<ScriptSubItem> script_subs;
};

std::unordered_map<std::string, Subscription> g_subscription;

void EventPort::postFromNative(const std::string& event, const kwui::ScriptValue& value)
{
	return doPost(event, value);
}
void EventPort::addListenerFromNative(const std::string& event, kwui::ScriptFunction* func, void* udata)
{
	Subscription& sub = g_subscription[event];
	
	// TODO: check duplicate add
	
	sub.native_subs.emplace_back<NativeSubItem>({ func, udata });
}
bool EventPort::removeListenerFromNative(const std::string& event, kwui::ScriptFunction* func, void* udata)
{
	auto it = g_subscription.find(event);
	if (it == g_subscription.end())
		return false;
	auto& native_subs = it->second.native_subs;
	for (auto& i = native_subs.begin(); i != native_subs.end(); ++i) {
		if (i->func == func && i->udata == udata) {
			native_subs.erase(i);
			return true;
		}
	}
	return false;
}
JSValue EventPort::postFromScript(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
	if (!JS_IsString(argv[0])) {
		return JS_ThrowTypeError(ctx, "post: expect string event");
	}
	const char* event = JS_ToCString(ctx, argv[0]);
	kwui::ScriptValue arg = (argc > 1) ? wrap(ctx, argv[1]) : kwui::ScriptValue();
	doPost(event, arg);
	JS_FreeCString(ctx, event);
	return JS_UNDEFINED;
}
JSValue EventPort::addListenerFromScript(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
	if (!JS_IsString(argv[0])) {
		return JS_ThrowTypeError(ctx, "addListener: expect string event");
	}
	const char* event = JS_ToCString(ctx, argv[0]);
	Subscription& sub = g_subscription[std::string(event)];
	JS_FreeCString(ctx, event);

	// TODO: check duplicate add

	sub.script_subs.emplace_back<ScriptSubItem>({ ctx, JS_DupValue(ctx, argv[1]) });
	return JS_UNDEFINED;
}
JSValue EventPort::removeListenerFromScript(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
	if (!JS_IsString(argv[0])) {
		return JS_ThrowTypeError(ctx, "addListener: expect string event");
	}
	const char* event_str = JS_ToCString(ctx, argv[0]);
	std::string event(event_str);
	Subscription& sub = g_subscription[std::string(event)];
	JS_FreeCString(ctx, event_str);
	
	auto it = g_subscription.find(event);
	if (it == g_subscription.end())
		return JS_FALSE;
	auto& script_subs = it->second.script_subs;
	for (auto& i = script_subs.begin(); i != script_subs.end(); ++i) {
		if (i->func == argv[1]) {
			JS_FreeValue(i->ctx, i->func);
			script_subs.erase(i);
			return JS_TRUE;
		}
	}
	return JS_FALSE;
}
void EventPort::setupAppObject(JSContext* ctx, JSValue app)
{
	JS_SetPropertyStr(ctx, app, "post",
		JS_NewCFunction(ctx, &EventPort::postFromScript, "app.post", 1));
	JS_SetPropertyStr(ctx, app, "addListener",
		JS_NewCFunction(ctx, &EventPort::addListenerFromScript, "app.addListener", 2));
	JS_SetPropertyStr(ctx, app, "removeListener",
		JS_NewCFunction(ctx, &EventPort::removeListenerFromScript, "app.removeListener", 2));
}
void EventPort::doPost(const std::string& event, const kwui::ScriptValue& value)
{
	auto it = g_subscription.find(event);
	if (it == g_subscription.end())
		return;
	Subscription sub = it->second;
	kwui::ScriptValue argv[2] = { event, value };
	for (auto& item : sub.native_subs) {
		item.func(2, argv, item.udata);
	}

	for (auto& item : sub.script_subs) {
		JSValue jval[2];
		jval[0] = JS_NewString(item.ctx, event.c_str());
		jval[1] = script::unwrap(item.ctx, value);
		JSValue ret = JS_Call(item.ctx, item.func, JS_UNDEFINED, 2, jval);
		if (JS_IsException(ret)) {
			js_std_dump_error(item.ctx);
		}
		JS_FreeValue(item.ctx, jval[0]);
		JS_FreeValue(item.ctx, jval[1]);
	}
}

}
