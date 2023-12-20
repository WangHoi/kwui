#include "Keact.h"
#include "ComponentState.h"
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
        return comp_state->useHook(ctx, comp_state_var, argv[0], argv[1]);
    } else {
        return JS_UNDEFINED;
    }
}

static int initModule(JSContext* ctx, JSModuleDef* m)
{
    JSValue use_hook;
    use_hook = JS_NewCFunction(ctx, useHook, "useHook", 2);
    //JS_SetModuleExport(ctx, m, "useHook", use_hook);
    auto global = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global, "useHook", use_hook);
    JS_FreeValue(ctx, global);
    return 0;
}

JSModuleDef* initModule(JSContext* ctx)
{
    JSValue use_hook;
    use_hook = JS_NewCFunction(ctx, useHook, "useHook", 2);
    //JS_SetModuleExport(ctx, m, "useHook", use_hook);
    auto global = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global, "useHook", use_hook);
    JS_FreeValue(ctx, global);
    return nullptr;

    JSModuleDef* m;
    m = JS_NewCModule(ctx, "Keact", initModule);
    if (!m)
        return NULL;
    JS_AddModuleExport(ctx, m, "useHook");
    return m;
}

}
}
