#include "EventPort.h"
#include "absl/base/macros.h"
#include <unordered_map>

namespace script {

struct NativeSubItem {
	kwui::ScriptFunction func;
	void* udata;
};

struct Subscription {
	std::vector<NativeSubItem> native_subs;
	std::vector<Value> script_subs;
};

std::unordered_map<std::string, Subscription> g_subscription;

void EventPort::postFromNative(const std::string& event, const kwui::ScriptValue& value)
{
	return doPost(event, value);
}
kwui::ScriptValue EventPort::sendFromNative(const std::string& event, const kwui::ScriptValue& value)
{
	kwui::ScriptValue ret = kwui::ScriptValue::newArray();
	size_t n = 0;
	doPost(event, value, [&](const kwui::ScriptValue& r) {
		ret[n++] = r;
		});
	return ret;
}
void EventPort::addListenerFromNative(const std::string& event, kwui::ScriptFunction func, void* udata)
{
	Subscription& sub = g_subscription[event];
	
	// TODO: check duplicate add
	
	sub.native_subs.emplace_back<NativeSubItem>({ func, udata });
}
bool EventPort::removeListenerFromNative(const std::string& event, kwui::ScriptFunction func, void* udata)
{
	auto it = g_subscription.find(event);
	if (it == g_subscription.end())
		return false;
	auto& native_subs = it->second.native_subs;
	bool removed = false;
	for (auto i = native_subs.begin(); i != native_subs.end(); ++i) {
		if (i->func == func && i->udata == udata) {
			native_subs.erase(i);
			removed = true;
			break;
		}
	}
	if (it->second.native_subs.empty() && it->second.script_subs.empty()) {
		g_subscription.erase(it);
	}
	return removed;
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

	sub.script_subs.emplace_back(ctx, argv[1]);
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
	bool removed = false;
	for (auto i = script_subs.begin(); i != script_subs.end(); ++i) {
		if (i->jsValue() == argv[1]) {
			script_subs.erase(i);
			removed = true;
			break;
		}
	}
	if (it->second.native_subs.empty() && it->second.script_subs.empty()) {
		g_subscription.erase(it);
	}
	return removed ? JS_TRUE : JS_FALSE;
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
void EventPort::doPost(const std::string& event, const kwui::ScriptValue& value,
	absl::optional<absl::FunctionRef<void(const kwui::ScriptValue&)>> fn)
{
	auto it = g_subscription.find(event);
	if (it == g_subscription.end()) {
		return;
	}
	Subscription sub = it->second;
	kwui::ScriptValue evt(event);
	const kwui::ScriptValue* argv[2] = { &evt, &value };
	for (auto& item : sub.native_subs) {
		kwui::ScriptValue ret = item.func(2, argv, item.udata);
		if (fn.has_value()) {
			fn.value()(ret);
		}
	}

	for (auto& item : sub.script_subs) {
		JSValue jval[2];
		jval[0] = JS_NewString(item.jsContext(), event.c_str());
		jval[1] = script::unwrap(item.jsContext(), value);
		JSValue ret = JS_Call(item.jsContext(), item.jsValue(), JS_UNDEFINED, 2, jval);
		if (JS_IsException(ret)) {
			js_std_dump_error(item.jsContext());
		}
		if (fn.has_value()) {
			fn.value()(script::wrap(item.jsContext(), ret));
		}
		JS_FreeValue(item.jsContext(), jval[0]);
		JS_FreeValue(item.jsContext(), jval[1]);
	}
}
void EventPort::cleanup()
{
	g_subscription.clear();
}

}
