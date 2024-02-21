#include "Keact.h"
#include "ComponentState.h"
#include "ContextId.h"
#include "absl/cleanup/cleanup.h"

namespace script {
namespace Keact {

static JSValue useHook(JSContext* ctx, JSValueConst /*this_val*/, int argc, JSValueConst* argv)
{
    auto global = JS_GetGlobalObject(ctx);
    auto comp_state_var = JS_GetPropertyStr(ctx, global, "__comp_state");
    absl::Cleanup _ = [&]() {
        JS_FreeValue(ctx, comp_state_var);
        JS_FreeValue(ctx, global);
        };
    
    auto comp_state = (ComponentState *)JS_GetOpaque2(ctx, comp_state_var, ComponentState::JS_CLASS_ID);
    if (comp_state) {
        JSValue cleanup_fn = (argc > 2) ? argv[2] : JS_UNDEFINED;
        return comp_state->useHook(ctx, comp_state_var, argv[0], argv[1], cleanup_fn);
    } else {
        return JS_ThrowInternalError(ctx, "useHook: no current Component");
    }
}

static JSValue createContextId(JSContext* ctx, JSValueConst /*this_val*/, int argc, JSValueConst* argv)
{
    return ContextId::create(ctx);
}

static JSValue provideContext(JSContext* ctx, JSValueConst /*this_val*/, int argc, JSValueConst* argv)
{
    auto global = JS_GetGlobalObject(ctx);
    auto comp_state_var = JS_GetPropertyStr(ctx, global, "__comp_state");
    absl::Cleanup _ = [&]() {
        JS_FreeValue(ctx, comp_state_var);
        JS_FreeValue(ctx, global);
        };

    auto comp_state = (ComponentState*)JS_GetOpaque2(ctx, comp_state_var, ComponentState::JS_CLASS_ID);
    if (comp_state) {
        return comp_state->provideContext(ctx, comp_state_var, argv[0], argv[1]);
    } else {
        return JS_ThrowInternalError(ctx, "__provideContext: no current Component");
    }
}

static JSValue useContext(JSContext* ctx, JSValueConst /*this_val*/, int argc, JSValueConst* argv)
{
    auto global = JS_GetGlobalObject(ctx);
    auto comp_state_var = JS_GetPropertyStr(ctx, global, "__comp_state");
    absl::Cleanup _ = [&]() {
        JS_FreeValue(ctx, comp_state_var);
        JS_FreeValue(ctx, global);
        };

    auto comp_state = (ComponentState*)JS_GetOpaque2(ctx, comp_state_var, ComponentState::JS_CLASS_ID);
    if (comp_state) {
        return comp_state->useContext(ctx, comp_state_var, argv[0]);
    } else {
        return JS_ThrowInternalError(ctx, "__useContext: no current Component");
    }
}

static JSValue useEffect(JSContext* ctx, JSValueConst /*this_val*/, int argc, JSValueConst* argv)
{
    auto global = JS_GetGlobalObject(ctx);
    auto comp_state_var = JS_GetPropertyStr(ctx, global, "__comp_state");
    absl::Cleanup _ = [&]() {
        JS_FreeValue(ctx, comp_state_var);
        JS_FreeValue(ctx, global);
        };

    auto comp_state = (ComponentState*)JS_GetOpaque2(ctx, comp_state_var, ComponentState::JS_CLASS_ID);
    if (comp_state) {
        return comp_state->useEffect(ctx, comp_state_var, argv[0], argv[1], argv[2]);
    } else {
        return JS_ThrowInternalError(ctx, "__useEffect: no current Component");
    }
}

//static int initModule(JSContext* ctx, JSModuleDef* m)
//{
//    auto global = JS_GetGlobalObject(ctx);
//    
//    JS_SetPropertyStr(ctx, global, "useHook", JS_NewCFunction(ctx, useHook, "useHook", 2));
//    JS_SetPropertyStr(ctx, global, "__createContextId", JS_NewCFunction(ctx, createContextId, "__createContextId", 0));
//    JS_SetPropertyStr(ctx, global, "__provideContext", JS_NewCFunction(ctx, provideContext, "__provideContext", 2));
//    JS_SetPropertyStr(ctx, global, "__useContext", JS_NewCFunction(ctx, useContext, "__useContext", 1));
//    JS_SetPropertyStr(ctx, global, "__useEffect", JS_NewCFunction(ctx, useEffect, "__useEffect", 3));
//    
//    JS_FreeValue(ctx, global);
//    return 0;
//}

JSModuleDef* initModule(JSContext* ctx)
{
    ContextId::registerClass(ctx);

    auto global = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global, "useHook", JS_NewCFunction(ctx, useHook, "useHook", 2));
    JS_SetPropertyStr(ctx, global, "__createContextId", JS_NewCFunction(ctx, createContextId, "__createContextId", 0));
    JS_SetPropertyStr(ctx, global, "__provideContext", JS_NewCFunction(ctx, provideContext, "__provideContext", 2));
    JS_SetPropertyStr(ctx, global, "__useContext", JS_NewCFunction(ctx, useContext, "__useContext", 1));
    JS_SetPropertyStr(ctx, global, "__useEffect", JS_NewCFunction(ctx, useEffect, "__useEffect", 3));
    JS_FreeValue(ctx, global);
    return nullptr;
    /*
    JSModuleDef* m;
    m = JS_NewCModule(ctx, "Keact", initModule);
    if (!m)
        return NULL;
    JS_AddModuleExport(ctx, m, "useHook");
    return m;
    */
}

}
}
