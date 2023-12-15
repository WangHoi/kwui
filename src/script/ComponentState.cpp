#include "ComponentState.h"
#include "absl/base/macros.h"
#include "absl/log/log.h"

namespace script {

static JSClassID g_component_state_clsid;

static void component_state_finalizer(JSRuntime* rt, JSValue val)
{
    //JSPointData* s = JS_GetOpaque(val, g_component_state_clsid);
    /* Note: 's' can be NULL in case JS_SetOpaque() was not called */
    //js_free_rt(rt, s);
    LOG(INFO) << "__ComponentState finalizer called";
}

// 	constructor(renderFn, props, children)
static JSValue component_state_ctor(JSContext* ctx,
    JSValueConst new_target,
    int argc, JSValueConst* argv)
{
    JSValue obj = JS_UNDEFINED;
    JSValue proto;

    if (!JS_IsFunction(ctx, argv[0]))
        goto fail;
    if (!JS_IsObject(argv[1]))
        goto fail;
    if (!JS_IsArray(ctx, argv[2]))
        goto fail;
    /* using new_target to get the prototype is necessary when the
       class is extended. */
    proto = JS_GetPropertyStr(ctx, new_target, "prototype");
    if (JS_IsException(proto))
        goto fail;
    obj = JS_NewObjectProtoClass(ctx, proto, g_component_state_clsid);
    JS_FreeValue(ctx, proto);
    if (JS_IsException(obj))
        goto fail;
    JS_SetPropertyStr(ctx, obj, "renderFn", JS_DupValue(ctx, argv[0]));
    JS_SetPropertyStr(ctx, obj, "props", JS_DupValue(ctx, argv[1]));
    JS_SetPropertyStr(ctx, obj, "children", JS_DupValue(ctx, argv[2]));
    return obj;
fail:
    JS_FreeValue(ctx, obj);
    return JS_EXCEPTION;
}

JSValue component_state_render(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
    JSValue render_fn, args[2];
    render_fn = JS_GetPropertyStr(ctx, this_val, "renderFn");
    args[0] = JS_GetPropertyStr(ctx, this_val, "props");
    args[1] = JS_GetPropertyStr(ctx, this_val, "children");
    return JS_Call(ctx, render_fn, this_val, 2, args);
}

static JSClassDef g_component_state_class = {
    "__ComponentState",
    component_state_finalizer, // finalizer
};

// TODO: remove duplicated func in script.cpp
inline JSCFunctionListEntry func_def(const char* name, uint8_t minargs, JSCFunction* pf)
{
    //#define JS_CFUNC_DEF(name, length, func1) { name, JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE, JS_DEF_CFUNC, 0, 
             //.u = { .func = { length, JS_CFUNC_generic, { .generic = func1 } } } }
    JSCFunctionListEntry def = { 0 };
    def.name = name;
    def.prop_flags = JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE;
    def.def_type = JS_DEF_CFUNC;
    def.magic = 0;
    def.u.func.length = minargs;
    def.u.func.cfunc.generic = pf;
    def.u.func.cproto = JS_CFUNC_generic;
    return def;
}

static const JSCFunctionListEntry g_component_state_funcs[1] = {
    func_def("render", 0, &component_state_render),
};

void ComponentState::registerClass(JSContext* ctx)
{
    JSValue proto, klass;

    /* create the Point class */
    JS_NewClassID(&g_component_state_clsid);
    JS_NewClass(JS_GetRuntime(ctx), g_component_state_clsid, &g_component_state_class);

    proto = JS_NewObject(ctx);
    JS_SetPropertyFunctionList(ctx, proto, g_component_state_funcs, ABSL_ARRAYSIZE(g_component_state_funcs));

    klass = JS_NewCFunction2(ctx,
        component_state_ctor,
        g_component_state_class.class_name,
        3,
        JS_CFUNC_constructor, 0);
    /* set proto.constructor and ctor.prototype */
    JS_SetConstructor(ctx, klass, proto);
    JS_SetClassProto(ctx, g_component_state_clsid, proto);

    JSValue global = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global, g_component_state_class.class_name, klass);
    JS_FreeValue(ctx, global);
}

JSValue ComponentState::newObject(JSContext* ctx, int argc, JSValueConst* argv)
{
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue klass = JS_GetPropertyStr(ctx, global, g_component_state_class.class_name);
    JSValue obj = JS_CallConstructor(ctx, klass, argc, argv);
    JS_FreeValue(ctx, global);
    return obj;
}

}