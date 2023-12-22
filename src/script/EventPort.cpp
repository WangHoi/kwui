#include "EventPort.h"
#include "absl/base/macros.h"

namespace script {

JSClassID EventPort::JS_CLASS_ID = 0;

static const char* js_event_port_clsname = "__EventPort";
static JSClassDef js_event_port_clsdef = {
	js_event_port_clsname,
	&EventPort::finalizer,
    &EventPort::gcMark,
};
static const JSCFunctionListEntry js_event_port_proto_funcs[] = {
	js_cfunc_def("addListener", 1, &EventPort::addListener),
    js_cfunc_def("removeListener", 1, &EventPort::removeListener),
};

static int g_next_port_id = 1;
static std::map<int, EventPort*> g_event_port_map;

void js_add_event_port(JSContext* ctx)
{
	JSValue global = JS_GetGlobalObject(ctx);

	JS_NewClassID(&EventPort::JS_CLASS_ID);
	JS_NewClass(JS_GetRuntime(ctx), EventPort::JS_CLASS_ID, &js_event_port_clsdef);

	JSValue proto = JS_NewObject(ctx);
	JS_SetPropertyFunctionList(ctx, proto, js_event_port_proto_funcs, ABSL_ARRAYSIZE(js_event_port_proto_funcs));

	JSValue klass = JS_NewCFunction2(ctx, &EventPort::constructor, js_event_port_clsname, 0, JS_CFUNC_constructor, 0);
    /* set proto.constructor and ctor.prototype */
    JS_SetConstructor(ctx, klass, proto);
    JS_SetClassProto(ctx, EventPort::JS_CLASS_ID, proto);

	JS_SetPropertyStr(ctx, global, js_event_port_clsname, klass);

	JS_FreeValue(ctx, global);
}

bool EventPort::postEvent(int port_id, const kwui::ScriptValue& val)
{
    auto it = g_event_port_map.find(port_id);
    if (it == g_event_port_map.end())
        return false;
    EventPort* port = it->second;
    std::vector<Value> listeners = port->listeners_;
    Value v(port->ctx_, val);
    for (const auto& func : listeners) {
        JS_Call(port->ctx_, func.jsValue(), JS_UNDEFINED, 1, &v.jsValue());
    }
    return true;
}

JSValue EventPort::constructor(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv)
{
    EventPort* port = nullptr;
    JSValue obj = JS_UNDEFINED;
    JSValue proto;

    if (g_next_port_id == 0) {
        return JS_ThrowInternalError(ctx, "event port exhausted.");
    }

    port = new EventPort();
    if (!port)
        return JS_EXCEPTION;
    /* using new_target to get the prototype is necessary when the
       class is extended. */
    proto = JS_GetPropertyStr(ctx, new_target, "prototype");
    if (JS_IsException(proto))
        goto fail;
    obj = JS_NewObjectProtoClass(ctx, proto, JS_CLASS_ID);
    JS_FreeValue(ctx, proto);
    if (JS_IsException(obj))
        goto fail;
    JS_SetOpaque(obj, port);

    port->ctx_ = ctx;
    port->id_ = g_next_port_id++;
    g_event_port_map[port->id_] = port;

    return obj;
fail:
    delete port;
    JS_FreeValue(ctx, obj);
    return JS_EXCEPTION;
}

void EventPort::finalizer(JSRuntime* rt, JSValue val)
{
	EventPort* port = (EventPort*)JS_GetOpaque(val, JS_CLASS_ID);
    g_event_port_map.erase(port->id_);
	delete port;
}

void EventPort::gcMark(JSRuntime* rt, JSValueConst val, JS_MarkFunc* mark_func)
{
    EventPort* port = (EventPort*)JS_GetOpaque(val, JS_CLASS_ID);
    for (const auto& l : port->listeners_) {
        JS_MarkValue(rt, l.jsValue(), mark_func);
    }
}

JSValue EventPort::addListener(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
    EventPort* port = (EventPort*)JS_GetOpaque(this_val, JS_CLASS_ID);
    auto it = std::find(port->listeners_.begin(), port->listeners_.end(), argv[0]);
    if (it == port->listeners_.end()) {
        port->listeners_.emplace_back(ctx, argv[0]);
    }
    return JS_TRUE;
}

JSValue EventPort::removeListener(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
    EventPort* port = (EventPort*)JS_GetOpaque(this_val, JS_CLASS_ID);
    auto it = std::find(port->listeners_.begin(), port->listeners_.end(), argv[0]);
    if (it == port->listeners_.end()) {
        return JS_FALSE;
    }
    port->listeners_.erase(it);
    return JS_TRUE;
}

}
