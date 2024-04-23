#include "script.h"
#include "scene2d/Scene.h"
#include "Dialog.h"
#ifdef _WIN32
#include "windows/DialogWin32.h"
#include "base/ResourceManager.h"
#include "base/EncodingManager.h"
#endif
#ifdef __ANDROID__
#include "android/DialogAndroid.h"
#endif
#include "Keact.h"
#include "resources/resources.h"
#include "absl/strings/str_format.h"
#include "absl/cleanup/cleanup.h"

namespace script {

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

JSMemoryUsage Runtime::computeMemoryUsage()
{
	JSMemoryUsage s = {};
	JS_ComputeMemoryUsage(rt_, &s);
	return s;
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

static std::tuple<std::string, size_t> cleanup_module_name(absl::string_view module_name)
{
	size_t dot_pos = module_name.rfind('.');
	if (dot_pos != absl::string_view::npos) {
		size_t tilde_pos = module_name.find('~', dot_pos);
		if (tilde_pos != absl::string_view::npos) {
			size_t version;
			if (absl::SimpleAtoi(module_name.substr(tilde_pos + 1), &version)) {
				std::string clean_module_name(module_name.substr(0, tilde_pos));
				return std::make_tuple(clean_module_name, 0);
			}
		}
	}
	return std::make_tuple(std::string(module_name), 0);
}

static JSModuleDef* load_module(JSContext* ctx, const char* orig_module_name, void* opaque)
{
	JSModuleDef* m;

	size_t buf_len = 0;
	uint8_t* buf = nullptr;
	JSValue func_val = JS_UNDEFINED;
	bool is_compiled = false;
	std::string module_name;
	std::tie(module_name, std::ignore) = cleanup_module_name(orig_module_name);

	if (absl::StartsWith(module_name, ":")) {
		auto u16_name = base::EncodingManager::UTF8ToWide(&module_name[1]);
		auto res_opt = base::ResourceManager::instance()->loadResource(u16_name.c_str());
		if (res_opt.has_value()) {
			buf_len = res_opt->size;
			buf = (uint8_t*)js_mallocz(ctx, res_opt->size + 1);
			memcpy(buf, res_opt->data, res_opt->size);
		}
	} else {
		auto builtin_module = resources::get_module_binary(module_name.c_str());
		if (builtin_module.has_value()) {
			buf_len = builtin_module.value().length();
			buf = (uint8_t*)js_mallocz(ctx, buf_len + 1);
			memcpy(buf, builtin_module.value().data(), buf_len);
			is_compiled = true;
		} else {
			buf = js_load_file(ctx, &buf_len, module_name.c_str());
		}
	}

	if (!buf) {
		JS_ThrowReferenceError(ctx, "could not load module filename '%s'",
			module_name.c_str());
		return NULL;
	}

	if (is_compiled) {
		func_val= JS_ReadObject(ctx, buf, buf_len, JS_READ_OBJ_BYTECODE);
	} else {
		/* compile the module */
		func_val = JS_Eval(ctx, (char*)buf, buf_len, module_name.c_str(),
			JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
	}
	js_free(ctx, buf);
	if (JS_IsException(func_val)) {
		js_std_dump_error(ctx);
		return NULL;
	}
	/* XXX: could propagate the exception */
	js_module_set_import_meta(ctx, func_val, true, false);
	/* the module is already referenced, so we must free it */
	m = (JSModuleDef*)JS_VALUE_GET_PTR(func_val);
	JS_FreeValue(ctx, func_val);
	return m;
}

static char* normalize_module(JSContext* ctx, const char* base_name, const char* name, void* opaque)
{
	auto script_ctx = (Context*)JS_GetContextOpaque(ctx);
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

	filename = (char*)js_malloc(ctx, len + strlen(name) + 1 + 1 + 32);
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
	
	size_t version = script_ctx ? script_ctx->reloadVersion() : 0;
	if (version > 0) {
		strcat(filename, "~");
		strcat(filename, std::to_string(version).c_str());
	}
	// LOG(INFO) << absl::StrFormat("normalize: base_name [%s] name [%s] -> filename [%s]", base_name, name, filename);
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

static JSValue app_show_dialog(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue app_close_dialog(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue app_closing_dialog(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue app_resize_dialog(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue app_get_dialog_hwnd(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue app_get_dialog_dpi_scale(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue app_load_resource(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

static JSValue css_func(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
	int64_t len = 0;
	JS_GetPropertyLength(ctx, &len, argv[0]);

	DCHECK(argc == len) << "css: mismatch args";

	JSValue val = JS_GetPropertyUint32(ctx, argv[0], 0);
	std::string text = Context::parse<std::string>(ctx, val);
	JS_FreeValue(ctx, val);

	for (int i = 1; i < (int)len; ++i) {
		JSValue v = JS_ToString(ctx, argv[i]);
		text += Context::parse<std::string>(ctx, v);
		JS_FreeValue(ctx, v);

		v = JS_GetPropertyUint32(ctx, argv[0], i);
		text += Context::parse<std::string>(ctx, v);
		JS_FreeValue(ctx, v);
	}

	return JS_NewStringLen(ctx, text.c_str(), text.length());
}

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
	JSValue global = JS_GetGlobalObject(ctx);
	JS_SetPropertyStr(ctx, global, "JSX", JS_NewCFunction(ctx, &jsx_func, "JSX", 3));
	JS_SetPropertyStr(ctx, global, "css", JS_NewCFunction(ctx, &css_func, "css", 1));
	JS_FreeValue(ctx, global);
}

Context::Context(Runtime* rt)
{
	ctx_ = JS_NewContext(rt->rt_);
	//LOG(INFO) << "JS_NewContext: " << ctx_;
	JS_SetContextOpaque(ctx_, this);
	js_init_module_std(ctx_, "std");
	js_init_module_os(ctx_, "os");
	js_std_add_helpers(ctx_, 0, nullptr);
	register_jsx_function(ctx_);
	ComponentState::registerClass(ctx_);
	ElementRef::registerClass(ctx_);
	Keact::initModule(ctx_);

	auto global_obj = JS_GetGlobalObject(ctx_);

	app_ = JS_NewObject(ctx_);
	JS_SetPropertyStr(ctx_, app_, "showDialog",
		JS_NewCFunction(ctx_, app_show_dialog, "app_show_dialog", 1));
	JS_SetPropertyStr(ctx_, app_, "closeDialog",
		JS_NewCFunction(ctx_, app_close_dialog, "app_close_dialog", 1));
	JS_SetPropertyStr(ctx_, app_, "closingDialog",
		JS_NewCFunction(ctx_, app_closing_dialog, "app_closing_dialog", 1));
	JS_SetPropertyStr(ctx_, app_, "resizeDialog",
		JS_NewCFunction(ctx_, app_resize_dialog, "app_resize_dialog", 3));
	JS_SetPropertyStr(ctx_, app_, "getDialogHwnd",
		JS_NewCFunction(ctx_, app_get_dialog_hwnd, "app_get_dialog_hwnd", 1));
	JS_SetPropertyStr(ctx_, app_, "getDialogDpiScale",
		JS_NewCFunction(ctx_, app_get_dialog_dpi_scale, "app_get_dialog_dpi_scale", 1));
	//JS_SetPropertyStr(ctx_, app_, "loadResource",
	//	JS_NewCFunction(ctx_, app_load_resource, "app_load_resource", 1));
	EventPort::setupAppObject(ctx_, app_);
	JS_SetPropertyStr(ctx_, global_obj, "app", app_);

	JS_FreeValue(ctx_, global_obj);

	rt->contexts_.push_back(this);
	for (auto& func : rt->new_ctx_funcs_) {
		func(this);
	}
}

Context::~Context()
{
	auto rt = Runtime::get();
	auto it = std::find(rt->contexts_.begin(), rt->contexts_.end(), this);
	if (it != rt->contexts_.end())
		rt->contexts_.erase(it);
	
	//LOG(INFO) << "JS_FreeContext: " << ctx_;
	JS_FreeContext(ctx_);
	ctx_ = nullptr;
}

void Context::loadFile(const std::string& fname)
{
	if (absl::StartsWith(fname, ":/")) {
		std::wstring u16_fname = base::EncodingManager::UTF8ToWide(fname.substr(1));
		auto res_opt = base::ResourceManager::instance()->loadResource(u16_fname.c_str());
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
	/*
	int eval_type = absl::EndsWithIgnoreCase(fname, ".mjs")
		? JS_EVAL_TYPE_MODULE
		: (JS_DetectModule(content.data(), content.size()) ? JS_EVAL_TYPE_MODULE : JS_EVAL_TYPE_GLOBAL);
	*/
	const int eval_type = JS_EVAL_TYPE_MODULE;
	JSValue ret = JS_Eval(ctx_, content.c_str(), content.length(), fname.c_str(), eval_type);
	if (JS_IsException(ret)) {
		js_std_dump_error(ctx_);
	}
	JS_FreeValue(ctx_, ret);
}

struct DialogLength {
	enum Type {
		Default,
		Fixed,
		Auto,
	};
	Type type = Default;
	float length = 0;
};

struct DialogPosAnchor {
	enum Type {
		Default,
		TopLeft,
		Center,
	};
	Type type = Default;
	scene2d::PointF pos;
	scene2d::RectF computeWindowGeometry(float client_width, float client_height, bool customFrame, bool popup)
	{
#ifdef _WIN32
		HMONITOR monitor;
		MONITORINFO info = { sizeof(MONITORINFO) };
		float dpi_scale = 1.0f;
		
		// get monitor info
		if (type == Default) {
			POINT p = {};
			::GetCursorPos(&p);
			monitor = ::MonitorFromPoint(p, MONITOR_DEFAULTTOPRIMARY);
			::GetMonitorInfoW(monitor, &info);
			pos.x = (float)((info.rcWork.left + info.rcWork.right) / 2);
			pos.y = (float)((info.rcWork.top + info.rcWork.bottom) / 2);
			type = Center;
		} else {
			monitor = ::MonitorFromPoint(POINT{}, MONITOR_DEFAULTTOPRIMARY);
			::GetMonitorInfoW(monitor, &info);
		}
		dpi_scale = windows::DialogWin32::getMonitorDpiScale(monitor);
		scene2d::RectF avail_rect = scene2d::RectF::fromLTRB(
			info.rcWork.left, info.rcWork.top, info.rcWork.right, info.rcWork.bottom);

		scene2d::RectF rect = scene2d::RectF::fromOriginSize(
			scene2d::PointF(),
			scene2d::DimensionF(client_width, client_height) * dpi_scale);
		rect = windows::DialogWin32::adjustWindowRect(rect, dpi_scale, customFrame, popup);

		if (type == TopLeft) {
			rect.moveTo(pos);
		} else if (type == Center) {
			rect.translate(avail_rect.center() - rect.center());
		}
	
		return rect;
#else
#pragma message("TODO: computeWindowGeometry not implemented.")
		return scene2d::RectF();
#endif
	}
};

struct ShowDialogConfig {
	std::string parent_dialog_id;
	absl::optional<std::string> title;
	DialogPosAnchor anchor;
	DialogLength width, height;
	Value root;
	Value stylesheet;
	Value module;
	Value module_params;
#ifdef _WIN32
	int flags = windows::DIALOG_FLAG_MAIN;
	absl::optional<windows::PopupShadowData> popup_shadow;
#endif
};

static absl::StatusOr<ShowDialogConfig> parse_show_dialog_config(JSContext* ctx, JSValue arg)
{
	ShowDialogConfig cfg;
	Context::eachObjectField(ctx, arg, [&](const char* name, JSValue value) {
		if (!strcmp(name, "title") && JS_IsString(value)) {
			cfg.title.emplace(Context::parse<std::string>(ctx, value));
		} else if (!strcmp(name, "width")) {
			if (JS_IsNumber(value)) {
				double f64;
				JS_ToFloat64(ctx, &f64, value);
				cfg.width.type = DialogLength::Fixed;
				cfg.width.length = (float)f64;
			} else if (JS_IsString(value)) {
				auto sval = Context::parse<std::string>(ctx, value);
				if (sval == "auto") {
					cfg.width.type = DialogLength::Auto;
				}
			}
		} else if (!strcmp(name, "height")) {
			if (JS_IsNumber(value)) {
				double f64;
				JS_ToFloat64(ctx, &f64, value);
				cfg.height.type = DialogLength::Fixed;
				cfg.height.length = (float)f64;
			} else if (JS_IsString(value)) {
				auto sval = Context::parse<std::string>(ctx, value);
				if (sval == "auto") {
					cfg.height.type = DialogLength::Auto;
				}
			}
		} else if (!strcmp(name, "anchor") && JS_IsObjectPlain(ctx, value)) {
			int32_t x, y;
			JSValue left, top, right, bottom;
			left = JS_GetPropertyStr(ctx, value, "left");
			top = JS_GetPropertyStr(ctx, value, "top");
			right = JS_GetPropertyStr(ctx, value, "right");
			bottom = JS_GetPropertyStr(ctx, value, "bottom");
			if (JS_IsNumber(left) && JS_IsNumber(top)) {
				JS_ToInt32(ctx, &x, left);
				JS_ToInt32(ctx, &y, top);
				cfg.anchor.type = DialogPosAnchor::TopLeft;
				cfg.anchor.pos.x = x;
				cfg.anchor.pos.y = y;
			} else {
				LOG(WARNING) << "anchor other than 'top-left' not implemented";
			}
			JS_FreeValue(ctx, left);
			JS_FreeValue(ctx, top);
			JS_FreeValue(ctx, right);
			JS_FreeValue(ctx, bottom);
#ifdef _WIN32
		} else if (!strcmp(name, "flags") && JS_IsNumber(value)) {
			int32_t i32;
			JS_ToInt32(ctx, &i32, value);
			cfg.flags = i32;
		} else if (!strcmp(name, "customFrame") && JS_IsObjectPlain(ctx, value)) {
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

			cfg.popup_shadow.emplace<windows::PopupShadowData>({ image, padding });
#endif
		} else if (!strcmp(name, "root")) {
			cfg.root = Value(ctx, value);
		} else if (!strcmp(name, "stylesheet")) {
			cfg.stylesheet = Value(ctx, value);
		} else if (!strcmp(name, "modulePath")) {
			cfg.module = Value(ctx, value);
		} else if (!strcmp(name, "moduleParams")) {
			cfg.module_params = Value(ctx, value);
		} else if (!strcmp(name, "parent")) {
			cfg.parent_dialog_id = Context::parse<std::string>(ctx, value);
		}
		});
	return cfg;
}

JSValue app_show_dialog(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
	absl::StatusOr<ShowDialogConfig> res = parse_show_dialog_config(ctx, argv[0]);
	if (!res.ok()) {
		return JS_ThrowInternalError(ctx, "invalid arguments.");
	}
	auto&& cfg = std::move(res.value());

	script::DialogInterface* dialog = nullptr;
#ifdef _WIN32
	dialog = new windows::DialogWin32(
		L"dialog", NULL, cfg.flags,
		cfg.popup_shadow, absl::nullopt);
#endif
#ifdef __ANDROID__
	dialog = new android::DialogAndroid();
#endif
	if (!dialog) {
		return JS_ThrowInternalError(ctx, "app_show_dialog not implemented.");
	}
	
	dialog->GetScene()->setStyleSheet(cfg.stylesheet.jsValue());
	dialog->GetScene()->createComponentNode(dialog->GetScene()->root(), cfg.root.jsValue());
	if (JS_IsString(cfg.module.jsValue())) {
		JSAtom base_filename_atom = JS_GetScriptOrModuleName(ctx, 1);
		JSValue base_filename_value = JS_AtomToString(ctx, base_filename_atom);
		absl::Cleanup _ = [&]() {
			JS_FreeAtom(ctx, base_filename_atom);
			JS_FreeValue(ctx, base_filename_value);
			};
		std::string base_filename = Context::parse<std::string>(ctx, base_filename_value);
		std::string module_path = Context::parse<std::string>(ctx, cfg.module.jsValue());
		dialog->GetScene()->setScriptModule(base_filename, module_path, cfg.module_params);
		dialog->GetScene()->reloadScriptModule();
	}

#ifdef _WIN32
	auto dialog_win32 = static_cast<windows::DialogWin32*>(dialog);
	if (kwui::Application::scriptReloadEnabled()) {
		dialog_win32->SetTitle(cfg.title.value_or(std::string("Untitled")) + " - (F5 to reload)");
	} else {
		dialog_win32->SetTitle(cfg.title.value_or(std::string("Untitled")));
	}
	//LOG(INFO) << "show dialog";
	if (cfg.flags & windows::DIALOG_FLAG_POPUP) {
		dialog_win32->SetPopupAnchor(windows::DialogWin32::findDialogById(cfg.parent_dialog_id));
	} else {
		dialog_win32->SetParent(windows::DialogWin32::findDialogById(cfg.parent_dialog_id));
	}
	
	float cwidth = 640, cheight = 480;
	if (cfg.width.type == DialogLength::Fixed) {
		cwidth = cfg.width.length;
	} else if (cfg.width.type == DialogLength::Auto) {
		auto [min_intrin_width, max_intrin_width] = dialog->GetScene()->intrinsicWidth();
		LOG(INFO) << absl::StrFormat("intrin width: min=%.0f, max=%.0f",
			min_intrin_width, max_intrin_width);
		cwidth = max_intrin_width;
	}
	if (cfg.height.type == DialogLength::Fixed) {
		cheight = cfg.height.length;
	} else if (cfg.height.type == DialogLength::Auto) {
		cheight = dialog->GetScene()->intrinsicHeight(cwidth);
		LOG(INFO) << absl::StrFormat("intrin height=%.0f for width: %.0f",
			cheight, cwidth);
	}
	auto rect = cfg.anchor.computeWindowGeometry(cwidth, cheight,
		cfg.popup_shadow.has_value(), (cfg.flags & windows::DIALOG_FLAG_POPUP));
	dialog_win32->setWindowPos(rect);
	dialog_win32->Show();
#endif
	return JS_NewString(ctx, dialog->eventContextId().c_str());
}
JSValue app_close_dialog(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
#ifdef _WIN32
	if (!JS_IsString(argv[0])) {
		return JS_ThrowTypeError(ctx, "closeDialog: expect id");
	}
	const char* id_str = JS_ToCString(ctx, argv[0]);
	std::string id(id_str);
	JS_FreeCString(ctx, id_str);
	auto dialog = windows::DialogWin32::findDialogById(id);
	if (dialog) {
		dialog->Close();
		delete dialog;
	}
	return JS_UNDEFINED;
#else
#pragma message("TODO: app_close_dialog not implemented.")
	return JS_ThrowInternalError(ctx, "app_close_dialog not implemented.");
#endif
}
JSValue app_closing_dialog(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
#ifdef _WIN32
	if (!JS_IsString(argv[0])) {
		return JS_ThrowTypeError(ctx, "closingDialog: expect id");
	}
	const char* id_str = JS_ToCString(ctx, argv[0]);
	std::string id(id_str);
	JS_FreeCString(ctx, id_str);
	auto dialog = windows::DialogWin32::findDialogById(id);
	if (dialog) {
		dialog->OnCloseSysCommand(*dialog);
	}
	return JS_UNDEFINED;
#else
#pragma message("TODO: app_closing_dialog not implemented.")
	return JS_ThrowInternalError(ctx, "app_closing_dialog not implemented.");
#endif
}
JSValue app_resize_dialog(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
#ifdef _WIN32
	if (!JS_IsString(argv[0])) {
		return JS_ThrowTypeError(ctx, "resizeDialog: expect id");
	}
	if (!JS_IsNumber(argv[1])) {
		return JS_ThrowTypeError(ctx, "resizeDialog: expect number width");
	}
	if (!JS_IsNumber(argv[2])) {
		return JS_ThrowTypeError(ctx, "resizeDialog: expect number height");
	}
	const char* id_str = JS_ToCString(ctx, argv[0]);
	std::string id(id_str);
	JS_FreeCString(ctx, id_str);
	double width, height;
	JS_ToFloat64(ctx, &width, argv[1]);
	JS_ToFloat64(ctx, &height, argv[2]);
	auto dialog = windows::DialogWin32::findDialogById(id);
	if (dialog)
		dialog->Resize((float)width, (float)height);
	return JS_UNDEFINED;
#else
#pragma message("TODO: app_resize_dialog not implemented.")
	return JS_ThrowInternalError(ctx, "app_resize_dialog not implemented.");
#endif
}
JSValue app_get_dialog_hwnd(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
#ifdef _WIN32
	if (!JS_IsString(argv[0])) {
		return JS_ThrowTypeError(ctx, "getDialogHwnd: expect id");
	}
	const char* id_str = JS_ToCString(ctx, argv[0]);
	std::string id(id_str);
	JS_FreeCString(ctx, id_str);
	auto dialog = windows::DialogWin32::findDialogById(id);
	if (dialog) {
		return JS_NewString(ctx, absl::StrFormat("%p", dialog->GetHwnd()).c_str());
	}
	return JS_UNDEFINED;
#else
#pragma message("TODO: app_get_dialog_hwnd not implemented.")
return JS_ThrowInternalError(ctx, "app_get_dialog_hwnd not implemented.");
#endif
}
JSValue app_get_dialog_dpi_scale(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
#ifdef _WIN32
	if (!JS_IsString(argv[0])) {
		return JS_ThrowTypeError(ctx, "getDialogDpiScale: expect id");
	}
	const char* id_str = JS_ToCString(ctx, argv[0]);
	std::string id(id_str);
	JS_FreeCString(ctx, id_str);
	auto dialog = windows::DialogWin32::findDialogById(id);
	if (dialog) {
		return JS_NewFloat64(ctx, dialog->GetDpiScale());
	}
	return JS_NewFloat64(ctx, 1.0);
#else
#pragma message("TODO: app_get_dialog_dpi_scale not implemented.")
	return JS_ThrowInternalError(ctx, "app_get_dialog_dpi_scale not implemented.");
#endif
}
JSValue app_load_resource(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
#ifdef _WIN32
	int32_t id = 0;
	int ret = JS_ToInt32(ctx, &id, argv[1]);
	if (ret == 0) {
		LOG(INFO) << "load resource with id " << id;
		base::ResourceManager::instance()->preloadResourceArchive(id);
	}
	return JS_UNDEFINED;
#else
#pragma message("TODO: app_load_resource not implemented.")
	return JS_ThrowInternalError(ctx, "app_load_resource not implemented.");
#endif
}

kwui::ScriptValue wrap(JSContext* ctx, JSValueConst c)
{
	if (JS_IsBool(c)) {
		return !!JS_ToBool(ctx, c);
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
