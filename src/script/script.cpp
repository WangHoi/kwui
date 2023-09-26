#include "script.h"
#include "scene2d/Scene.h"

namespace script {

inline JSCFunctionListEntry func_def(const char* name, byte minargs, JSCFunction* pf)
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

static JSClassID scene_class_id = 0;
static void scene_finalizer(JSRuntime* rt, JSValue val);
static void scene_gcmarker(JSRuntime* rt, JSValueConst val,
    JS_MarkFunc* mark_func);
static JSValue scene_render(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue scene_update_component(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

static JSClassDef scene_class_def = {
    "Scene",
    scene_finalizer,
    scene_gcmarker,
    nullptr,
    nullptr,
};
static const JSCFunctionListEntry scene_methods[] = {
    func_def("render", 1, scene_render),
    func_def("updateComponent", 1, scene_update_component),
};
void Context::initSceneClass()
{
    JS_NewClassID(&scene_class_id);
    JS_NewClass(JS_GetRuntime(ctx_), scene_class_id, &scene_class_def);
    JSValue proto = JS_NewObject(ctx_);
    JS_SetPropertyFunctionList(ctx_, proto, scene_methods, _countof(scene_methods));
    JS_FreeValue(ctx_, proto);
}

JSValue Context::wrapScene(scene2d::Scene* scene)
{
    JSValue j = JS_NewObjectClass(ctx_, scene_class_id);
    auto scene_weakptr = scene->weakObject();
    scene_weakptr->retain();
    JS_SetOpaque(j, scene_weakptr);
    return j;
}

void scene_finalizer(JSRuntime* rt, JSValue val)
{
    auto scene_weakptr = (base::WeakObject<scene2d::Scene> *)JS_GetOpaque(val, scene_class_id);
    scene_weakptr->release();
}

void scene_gcmarker(JSRuntime* rt, JSValueConst val, JS_MarkFunc* mark_func)
{
    // TODO: mark Components in scene
}

JSValue scene_render(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
    return JS_EXCEPTION;
}

JSValue scene_update_component(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
    auto scene_weakptr = (base::WeakObject<scene2d::Scene> *)JS_GetOpaque(this_val, scene_class_id);
    auto scene = scene_weakptr->get();
    if (!scene)
        return JS_UNDEFINED;
    scene->updateComponent(argv[1]);
}

static JSValue app_show_dialog(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

Context::Context(Runtime* rt)
{
    ctx_ = JS_NewContext(rt->rt_);
    JS_SetContextOpaque(ctx_, this);
    js_init_module_std(ctx_, "std");
    js_init_module_os(ctx_, "os");
    js_std_add_helpers(ctx_, 0, nullptr);
    initSceneClass();

    auto global_obj = JS_GetGlobalObject(ctx_);

    auto app = JS_NewObject(ctx_);
    JS_SetPropertyStr(ctx_, app, "showDialog",
        JS_NewCFunction(ctx_, app_show_dialog, "app_show_dialog", 1));
    JS_SetPropertyStr(ctx_, global_obj, "app", app);

    JS_FreeValue(ctx_, global_obj);
}

Context::~Context()
{
    JS_FreeContext(ctx_);
    ctx_ = nullptr;
}


JSValue app_show_dialog(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
    return JS_NewInt32(ctx, 12345);
}


}