#include "script.h"
#include "scene2d/Scene.h"
#include "windows/Dialog.h"
#include "windows/ResourceManager.h"
#include "Keact.h"

namespace script {

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
    js_cfunc_def("render", 1, scene_render),
    js_cfunc_def("updateComponent", 1, scene_update_component),
};

static Runtime* g_rt = nullptr;

Runtime* Runtime::createInstance()
{
    if (!g_rt)
        g_rt = new Runtime();
    return g_rt;
}

void Runtime::releaseInstance()
{
    if (g_rt) {
        delete g_rt;
        g_rt = nullptr;
    }
}

Runtime* Runtime::get()
{
    return g_rt;
}

void Runtime::gc()
{
    JS_RunGC(rt_);
}

void Runtime::addContextSetupFunc(std::function<void(Context*)>&& new_ctx_func)
{
    new_ctx_funcs_.emplace_back(std::move(new_ctx_func));
}

void Runtime::eachContext(absl::FunctionRef<void(Context*)> func)
{
    for (Context* ctx : contexts_) {
        func(ctx);
    }
}

Runtime::Runtime()
{
    rt_ = JS_NewRuntime();
    JS_SetRuntimeOpaque(rt_, this);
}

Runtime::~Runtime()
{
    JS_FreeRuntime(rt_);
    rt_ = nullptr;
}

void Context::initSceneClass()
{
    JS_NewClassID(&scene_class_id);
    JS_NewClass(JS_GetRuntime(ctx_), scene_class_id, &scene_class_def);
    JSValue proto = JS_NewObject(ctx_);
    JS_SetPropertyFunctionList(ctx_, proto, scene_methods, _countof(scene_methods));
    JS_FreeValue(ctx_, proto);
}

void scene_finalizer(JSRuntime* rt, JSValue val)
{
    auto scene_weakptr = (base::WeakObjectProxy<scene2d::Scene> *)JS_GetOpaque(val, scene_class_id);
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
    auto scene_weakptr = (base::WeakObjectProxy<scene2d::Scene> *)JS_GetOpaque(this_val, scene_class_id);
    auto scene = scene_weakptr->get();
    if (!scene)
        return JS_UNDEFINED;
    scene->updateComponentNode(nullptr, argv[1]);
    return JS_UNDEFINED;
}

static JSValue app_show_dialog(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue app_load_resource(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

static JSValue jsx_func(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
    /*
    function JSX(tag, atts, kids) {
	    if (typeof tag == 'function') {
		    return new __ComponentState__(tag, atts, kids);
	    } else {
		    return {
			    tag,
			    atts,
			    kids,
		    };
	    }
    }
    */
    if (JS_IsFunction(ctx, argv[0])) {
        return ComponentState::newObject(ctx, argc, argv);
    } else {
        JSValue obj = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, obj, "tag", JS_DupValue(ctx, argv[0]));
        JS_SetPropertyStr(ctx, obj, "atts", JS_DupValue(ctx, argv[1]));
        JS_SetPropertyStr(ctx, obj, "kids", JS_DupValue(ctx, argv[2]));
        return obj;
    }
}

static void register_jsx_function(JSContext* ctx)
{
    JSValue jsx = JS_NewCFunction(ctx, &jsx_func, "JSX", 3);
    JSValue global = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global, "JSX", jsx);
    JS_FreeValue(ctx, global);
}

Context::Context(Runtime* rt)
{
    ctx_ = JS_NewContext(rt->rt_);
    JS_SetContextOpaque(ctx_, this);
    js_init_module_std(ctx_, "std");
    js_init_module_os(ctx_, "os");
    js_std_add_helpers(ctx_, 0, nullptr);
    initSceneClass();
    register_jsx_function(ctx_);
    ComponentState::registerClass(ctx_);
    Keact::initModule(ctx_);
    js_add_event_port(ctx_);

    auto global_obj = JS_GetGlobalObject(ctx_);

    app_ = JS_NewObject(ctx_);
    JS_SetPropertyStr(ctx_, app_, "showDialog",
        JS_NewCFunction(ctx_, app_show_dialog, "app_show_dialog", 1));
    JS_SetPropertyStr(ctx_, app_, "loadResource",
        JS_NewCFunction(ctx_, app_load_resource, "app_load_resource", 1));
    JS_SetPropertyStr(ctx_, global_obj, "app", app_);

    JS_FreeValue(ctx_, global_obj);

    rt->contexts_.push_back(this);
    for (auto& func : rt->new_ctx_funcs_) {
        rt->eachContext(func);
    }
}

Context::~Context()
{
    JS_FreeContext(ctx_);
    ctx_ = nullptr;
}

void Context::loadFile(const std::string& fname)
{
    FILE* f = fopen(fname.c_str(), "rb");
    if (!f)
        return;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    if (size <= 0)
        return;
    std::string content;
    content.resize(size);
    fseek(f, 0, SEEK_SET);
    fread(content.data(), 1, size, f);
    int eval_type = absl::EndsWithIgnoreCase(fname, ".mjs")
        ? JS_EVAL_TYPE_MODULE
        : (JS_DetectModule(content.data(), content.size()) ? JS_EVAL_TYPE_MODULE : JS_EVAL_TYPE_GLOBAL);
    JSValue ret = JS_Eval(ctx_, content.data(), size, fname.c_str(), eval_type);
    if (JS_IsException(ret)) {
        js_std_dump_error(ctx_);
    }
    JS_FreeValue(ctx_, ret);
}

JSValue Context::wrapScene(scene2d::Scene* scene)
{
    JSValue j = JS_NewObjectClass(ctx_, scene_class_id);
    auto scene_weakptr = scene->weakProxy();
    scene_weakptr->retain();
    JS_SetOpaque(j, scene_weakptr);
    return j;
}

JSValue app_show_dialog(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
    auto dialog = new windows::Dialog(
        640, 480, L"dialog", NULL, windows::DIALOG_FLAG_MAIN,
        absl::nullopt, absl::nullopt);
    if (argc >= 2 && JS_IsObject(argv[1])) {
        auto scene = dialog->GetScene();
        Context::eachObjectField(ctx, argv[1], [ctx, scene](const char *name, JSValue value) {
            auto selectors_res = style::Selector::parseGroup(name);
            auto style_spec = Context::parse<style::StyleSpec>(ctx, value);
            if (selectors_res.ok()) {
                for (auto&& selector : *selectors_res) {
                    auto rule = std::make_unique<style::StyleRule>(std::move(selector), style_spec);
                    scene->appendStyleRule(std::move(rule));
                }
            } else {
                LOG(WARNING) << "parse selector '" << name << "' failed";
            }
        });
    } else if (argc >= 2 && JS_IsString(argv[1])) {
        auto scene = dialog->GetScene();
        const char* s = JS_ToCString(ctx, argv[1]);
        if (s) {
            auto css_res = style::parse_css(s);
            if (css_res.ok()) {
                for (auto&& rule : css_res.value()) {
                    scene->appendStyleRule(std::move(rule));
                }
            }
        }
        JS_FreeCString(ctx, s);
    }
    scene2d::Node* content_root = dialog->GetScene()->createComponentNode(argv[0]);
    if (content_root)
        dialog->GetScene()->root()->appendChild(content_root);
    LOG(INFO) << "show dialog";
    dialog->Show();
    return JS_UNDEFINED;
}

JSValue app_load_resource(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
    int32_t id = 0;
    int ret = JS_ToInt32(ctx, &id, argv[1]);
    if (ret == 0) {
        LOG(INFO) << "load resource with id " << id;
        windows::ResourceManager::instance()->preloadResourceArchive(id);
    }
    return JS_UNDEFINED;
}

}
