#include "script.h"
#include "scene2d/Scene.h"
#include "windows/Dialog.h"
#include "windows/ResourceManager.h"
#include "windows/EncodingManager.h"
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

static JSModuleDef* load_module(JSContext* ctx, const char* module_name, void* opaque)
{
	JSModuleDef* m;

	size_t buf_len = 0;
	uint8_t* buf = nullptr;
	JSValue func_val = JS_UNDEFINED;

	if (module_name[0] == ':') {
		auto u16_name = windows::EncodingManager::UTF8ToWide(module_name + 1);
		auto res_opt = windows::ResourceManager::instance()->LoadResource(u16_name.c_str());
		if (res_opt.has_value()) {
			buf_len = res_opt->size;
			buf = (uint8_t*)js_mallocz(ctx, res_opt->size + 1);
			memcpy(buf, res_opt->data, res_opt->size);
		}
	} else {
		buf = js_load_file(ctx, &buf_len, module_name);
	}

	if (!buf) {
		JS_ThrowReferenceError(ctx, "could not load module filename '%s'",
			module_name);
		return NULL;
	}

	/* compile the module */
	func_val = JS_Eval(ctx, (char*)buf, buf_len, module_name,
		JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
	js_free(ctx, buf);
	if (JS_IsException(func_val))
		return NULL;
	/* XXX: could propagate the exception */
	js_module_set_import_meta(ctx, func_val, TRUE, FALSE);
	/* the module is already referenced, so we must free it */
	m = (JSModuleDef*)JS_VALUE_GET_PTR(func_val);
	JS_FreeValue(ctx, func_val);

	return m;
}

static char* normalize_module(JSContext* ctx, const char* base_name, const char* name, void* opaque)
{
	char* filename, * p;
	const char* r;
	int len;

	if (name[0] != '.') {
		/* if no initial dot, the module name is not modified */
		return js_strdup(ctx, name);
	}

	p = (char*)strrchr(base_name, '/');
	if (p)
		len = p - base_name;
	else
		len = 0;

	filename = (char*)js_malloc(ctx, len + strlen(name) + 1 + 1);
	if (!filename)
		return NULL;
	memcpy(filename, base_name, len);
	filename[len] = '\0';

	/* we only normalize the leading '..' or '.' */
	r = name;
	for (;;) {
		if (r[0] == '.' && r[1] == '/') {
			r += 2;
		} else if (r[0] == '.' && r[1] == '.' && r[2] == '/') {
			/* remove the last path element of filename, except if "."
			   or ".." */
			if (filename[0] == '\0')
				break;
			p = strrchr(filename, '/');
			if (!p)
				p = filename;
			else
				p++;
			if (!strcmp(p, ".") || !strcmp(p, ".."))
				break;
			if (p > filename)
				p--;
			*p = '\0';
			r += 3;
		} else {
			break;
		}
	}
	if (filename[0] != '\0')
		strcat(filename, "/");
	strcat(filename, r);
	//    printf("normalize: %s %s -> %s\n", base_name, name, filename);
	return filename;
}

Runtime::Runtime()
{
	rt_ = JS_NewRuntime();
	JS_SetRuntimeOpaque(rt_, this);
	JS_SetModuleLoaderFunc(rt_, &normalize_module, &load_module, this);
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

	auto global_obj = JS_GetGlobalObject(ctx_);

	app_ = JS_NewObject(ctx_);
	JS_SetPropertyStr(ctx_, app_, "showDialog",
		JS_NewCFunction(ctx_, app_show_dialog, "app_show_dialog", 1));
	//JS_SetPropertyStr(ctx_, app_, "loadResource",
	//	JS_NewCFunction(ctx_, app_load_resource, "app_load_resource", 1));
	EventPort::setupAppObject(ctx_, app_);
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
	if (absl::StartsWith(fname, ":/")) {
		std::wstring u16_fname = windows::EncodingManager::UTF8ToWide(fname.substr(1));
		auto res_opt = windows::ResourceManager::instance()->LoadResource(u16_fname.c_str());
		if (res_opt.has_value() && res_opt->data) {
			std::string content((const char*)res_opt->data, res_opt->size);
			loadScript(fname, content);
		} else {
			LOG(WARNING) << "Load file [" << fname << "] from resource failed.";
			return;
		}
	} else {
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
		fclose(f);

		loadScript(fname, content);
	}
}

void Context::loadScript(const std::string& fname, const std::string& content)
{
	int eval_type = absl::EndsWithIgnoreCase(fname, ".mjs")
		? JS_EVAL_TYPE_MODULE
		: (JS_DetectModule(content.data(), content.size()) ? JS_EVAL_TYPE_MODULE : JS_EVAL_TYPE_GLOBAL);
	JSValue ret = JS_Eval(ctx_, content.c_str(), content.length(), fname.c_str(), eval_type);
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
	float width = 640.0;
	float height = 480.0;
	int flags = windows::DIALOG_FLAG_MAIN;
	absl::optional<windows::PopupShadowData> popup_shadow = absl::nullopt;
	JSValue root = JS_UNDEFINED;
	JSValue stylesheet = JS_UNDEFINED;
	Context::eachObjectField(ctx, argv[0], [&](const char* name, JSValue value) {
		if (!strcmp(name, "width") && JS_IsNumber(value)) {
			double f64;
			JS_ToFloat64(ctx, &f64, value);
			width = (float)f64;
		} else if (!strcmp(name, "height") && JS_IsNumber(value)) {
			double f64;
			JS_ToFloat64(ctx, &f64, value);
			height = (float)f64;
		} else if (!strcmp(name, "flags") && JS_IsNumber(value)) {
			int32_t i32;
			JS_ToInt32(ctx, &i32, value);
			flags = i32;
		} else if (!strcmp(name, "popup_shadow") && JS_IsObjectPlain(ctx, value)) {
			std::string image;
			int padding = 0;
			JSValue val;

			val = JS_GetPropertyStr(ctx, value, "image");
			if (JS_IsString(val)) {
				size_t image_len = 0;
				const char* image_str = JS_ToCStringLen(ctx, &image_len, val);
				image.assign(image_str, image_len);
				JS_FreeCString(ctx, image_str);
			}
			JS_FreeValue(ctx, val);

			val = JS_GetPropertyStr(ctx, value, "padding");
			if (JS_IsNumber(val)) {
				int32_t i32;
				JS_ToInt32(ctx, &i32, val);
				padding = i32;
			}
			JS_FreeValue(ctx, val);

			popup_shadow.emplace<windows::PopupShadowData>({ image, padding });
		} else if (!strcmp(name, "root")) {
			root = JS_DupValue(ctx, value);
		} else if (!strcmp(name, "stylesheet")) {
			stylesheet = JS_DupValue(ctx, value);
		}
		});

	auto dialog = new windows::Dialog(
		width, height, L"dialog", NULL, flags,
		popup_shadow, absl::nullopt);
	if (JS_IsObject(stylesheet)) {
		auto scene = dialog->GetScene();
		Context::eachObjectField(ctx, argv[1], [ctx, scene](const char* name, JSValue value) {
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
	} else if (JS_IsString(stylesheet)) {
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
	scene2d::Node* content_root = dialog->GetScene()->createComponentNode(root);
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

kwui::ScriptValue wrap(JSContext* ctx, JSValueConst c)
{
	if (JS_IsBool(c)) {
		return JS_ToBool(ctx, c);
	} else if (JS_IsNumber(c)) {
		double f64 = 0.0;
		JS_ToFloat64(ctx, &f64, c);
		return kwui::ScriptValue(f64);
	} else if (JS_IsString(c)) {
		const char* s = nullptr;
		size_t len = 0;
		s = JS_ToCStringLen(ctx, &len, c);
		std::string str(s, len);
		JS_FreeCString(ctx, s);
		return kwui::ScriptValue(str);
	} else if (JS_IsArray(ctx, c)) {
		kwui::ScriptValue arr = kwui::ScriptValue::newArray();
		int64_t len = 0;
		JS_GetPropertyLength(ctx, &len, c);
		for (int i = 0; i < len; ++i) {
			JSValue elem = JS_GetPropertyUint32(ctx, c, (uint32_t)i);
			arr[i] = wrap(ctx, elem);
			JS_FreeValue(ctx, elem);
		}
		return arr;
	} else if (JS_IsObjectPlain(ctx, c)) {
		kwui::ScriptValue obj = kwui::ScriptValue::newObject();
		script::Context::eachObjectField(ctx, c, [&](const char* prop_name, JSValue prop_value) {
			obj[prop_name] = wrap(ctx, prop_value);
			});
		return obj;
	}
	return kwui::ScriptValue();
}

JSValue unwrap(JSContext* ctx, const kwui::ScriptValue& c)
{
	if (c.isBool()) {
		return JS_NewBool(ctx, c.toBool());
	} else if (c.isNumber()) {
		return JS_NewFloat64(ctx, c.toDouble());
	} else if (c.isString()) {
		std::string s = c.toString();
		return JS_NewStringLen(ctx, s.data(), s.length());
	} else if (c.isArray()) {
		JSValue arr = JS_NewArray(ctx);
		c.visitArray([&](int idx, const kwui::ScriptValue& elem) {
			JS_DefinePropertyValueUint32(ctx, arr, (uint32_t)idx, unwrap(ctx, elem), JS_PROP_C_W_E);
			});
		return arr;
	} else if (c.isObject()) {
		JSValue obj = JS_NewObject(ctx);
		c.visitObject([&](const std::string& key, const kwui::ScriptValue& elem) {
			JS_DefinePropertyValueStr(ctx, obj, key.c_str(), unwrap(ctx, elem), JS_PROP_C_W_E);
			});
		return obj;
	} else {
		return JS_NULL;
	}
}

}
