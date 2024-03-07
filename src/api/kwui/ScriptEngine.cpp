#include "ScriptEngine.h"
#include "script/script.h"
#include "absl/base/call_once.h"
#include "absl/functional/bind_front.h"
#include "base/log.h"
#include <vector>
#include <string>

namespace kwui {

struct FunctionDef {
	std::string name;
	ScriptFunction func;
	void* udata;
};

struct ScriptFunctionClosure {
	ScriptFunction func;
	void* udata;
};

static ScriptEngine* g_engine = nullptr;

class ScriptEngine::Private {
public:
	Private(ScriptEngine* se)
		: q(se), default_ctx(std::make_unique<script::Context>(script::Runtime::createInstance()))
	{
		script::Runtime::get()->addContextSetupFunc(absl::bind_front(&Private::initContext, this));
	}
	~Private()
	{
		default_ctx = nullptr;
		script::Runtime::releaseInstance();
	}
	void initContext(script::Context* ctx)
	{
		for (auto& def : global_funcs) {
			setGlobalFunction(def.get(), ctx);
		}
	}
	void setGlobalFunction(const FunctionDef* def, script::Context* ctx)
	{
		JSContext* j = ctx->get();
		JSValue global = JS_GetGlobalObject(j);
		
		JS_NewClassID(&script_func_clsid);
		JS_NewClass(JS_GetRuntime(j), script_func_clsid, &script_func_class);
		JSValue jobj = JS_NewObjectClass(j, script_func_clsid);
		auto closure = new ScriptFunctionClosure{ def->func, def->udata };
		JS_SetOpaque(jobj, closure);

		JS_SetPropertyStr(j, global, def->name.c_str(), jobj);
		
		JS_FreeValue(j, global);
	}
	
	void removeGlobalFunction(const char* name, script::Context* ctx)
	{
		JSContext* j = ctx->get();
		JSValue global = JS_GetGlobalObject(j);

		JS_SetPropertyStr(j, global, name, JS_UNDEFINED);

		JS_FreeValue(j, global);
	}
	static JSValue callScriptFunc(
		JSContext* ctx, JSValueConst func_obj,
		JSValueConst this_val, int argc, JSValueConst* argv,
		int flags)
	{
		auto closure = (ScriptFunctionClosure*)JS_GetOpaque2(ctx, func_obj, script_func_clsid);

		std::vector<ScriptValue> args;
		args.reserve(argc + 1);
		for (int i = 0; i < argc; ++i) {
			args.emplace_back(std::move(script::wrap(ctx, argv[i])));
		}
		args.emplace_back();

		std::vector<const ScriptValue*> pargs;
		pargs.reserve(args.size());
		for (auto& arg : args) {
			pargs.push_back(&arg);
		}
		ScriptValue ret = closure->func(argc, pargs.data(), closure->udata);

		return script::unwrap(ctx, ret);
	}
	static void script_func_finalizer(JSRuntime* rt, JSValue val);
	static JSClassID script_func_clsid;
	static JSClassDef script_func_class;

	ScriptEngine* q = nullptr;
	std::unique_ptr<script::Context> default_ctx;
	std::vector<std::unique_ptr<FunctionDef>> global_funcs;
	//std::vector<std::unique_ptr<ModuleRegister>> modules;
};

void ScriptEngine::Private::script_func_finalizer(JSRuntime* rt, JSValue val)
{
	auto closure = (ScriptFunctionClosure*)JS_GetOpaque(val, script_func_clsid);
	delete closure;
}

JSClassID ScriptEngine::Private::script_func_clsid = {};
JSClassDef ScriptEngine::Private::script_func_class = {
	"ScriptFunction",
	&Private::script_func_finalizer, // finalizer
	nullptr, // gcmark
	&ScriptEngine::Private::callScriptFunc // call
};

ScriptEngine* ScriptEngine::get()
{
	ABSL_CONST_INIT static absl::once_flag g_once;

	absl::call_once(g_once, [&]() {
		g_engine = new ScriptEngine;
		});

	return g_engine;
}

void ScriptEngine::release()
{
	delete g_engine;
	g_engine = nullptr;
}

void ScriptEngine::addGlobalFunction(const char* name, ScriptFunction func, void* udata)
{
	CHECK(Application::isMainThread())
		<< "addGlobalFunction() must be invoked from main thread";
	d->global_funcs.emplace_back(std::make_unique<FunctionDef>(FunctionDef{ name, func, udata }));
	script::Runtime::get()->eachContext(absl::bind_front(
		&Private::setGlobalFunction, d, d->global_funcs.back().get()));
}

void ScriptEngine::removeGlobalFunction(const char* name)
{
	CHECK(Application::isMainThread())
		<< "removeGlobalFunction() must be invoked from main thread";
	auto it = std::find_if(d->global_funcs.begin(), d->global_funcs.end(), [&](const auto& def) {
		return def->name == name;
		});
	if (it != d->global_funcs.end()) {
		d->global_funcs.erase(it);
		script::Runtime::get()->eachContext(absl::bind_front(
			&Private::removeGlobalFunction, d, name));
	}
}

void ScriptEngine::loadFile(const char* path)
{
	CHECK(Application::isMainThread())
		<< "loadFile() must be invoked from main thread";
	d->default_ctx->loadFile(path);
}

ScriptValue ScriptEngine::callGlobalFunction(const char* name, int argc, const ScriptValue* argv[])
{
	CHECK(Application::isMainThread())
		<< "callGlobalFunction() must be invoked from main thread";
	JSContext* ctx = d->default_ctx->get();
	JSValue global = JS_GetGlobalObject(ctx);
	JSValue func = JS_GetPropertyStr(ctx, global, name);
	ScriptValue ret;
	if (JS_IsFunction(ctx, func)) {
		std::vector<JSValue> jargs;
		for (int i = 0; i < argc; ++i) {
			jargs.push_back(script::unwrap(ctx, *argv[i]));
		}
		JSValue jret = JS_Call(ctx, func, JS_UNDEFINED, argc, jargs.data());
		if (JS_IsException(jret)) {
			js_std_dump_error(ctx);
		}
		ret = script::wrap(ctx, jret);
		JS_FreeValue(ctx, jret);
		for (int i = 0; i < argc; ++i) {
			JS_FreeValue(ctx, jargs[i]);
		}
	}
	JS_FreeValue(ctx, func);
	JS_FreeValue(ctx, global);
	return ret;
}

//ScriptEngine::ModuleRegister& ScriptEngine::addGlobalModule(const char* name)
//{
//	d->modules.emplace_back(std::make_unique<ModuleRegister>());
//	return *d->modules.back();
//}

void ScriptEngine::postEvent(const std::string& event, const ScriptValue& value)
{
	if (!Application::isMainThread()) {
		Application::runInMainThread([=] {
			script::EventPort::postFromNative(event, value);
			});
		return;
	}
	script::EventPort::postFromNative(event, value);
}

ScriptValue ScriptEngine::sendEvent(const std::string& event, const ScriptValue& value)
{
	CHECK(Application::isMainThread())
		<< "sendEvent() must be invoked from main thread";
	return script::EventPort::sendFromNative(event, value);
}

void ScriptEngine::addEventListener(const std::string& event,
	ScriptFunction func, void* udata)
{
	if (!Application::isMainThread()) {
		Application::runInMainThread([=] {
			script::EventPort::addListenerFromNative(event, func, udata);
			});
		return;
	}
	script::EventPort::addListenerFromNative(event, func, udata);
}

void ScriptEngine::removeEventListener(const std::string& event,
	ScriptFunction func, void* udata)
{
	if (!Application::isMainThread()) {
		Application::runInMainThread([=] {
			ScriptEngine::removeEventListener(event, func, udata);
			});
		return;
	}
	script::EventPort::removeListenerFromNative(event, func, udata);
}

ScriptEngine::ScriptEngine()
	: d(new Private(this))
{
}
ScriptEngine::~ScriptEngine()
{
	delete d;
}
/*
class ScriptEngine::ModuleRegister::Private {
public:
	Private(ModuleRegister* mr)
		: q(mr) {}

	ModuleRegister* q = nullptr;
	std::vector<std::unique_ptr<FunctionDef>> funcs;
};

void ScriptEngine::ModuleRegister::addFunction(const char* name, int min_args, ScriptFunction func)
{
	d->funcs.emplace_back(std::make_unique<FunctionDef>(name, min_args, func));
}

void ScriptEngine::ModuleRegister::addProperty(const char* name, ScriptPropertyGetter getter, ScriptPropertySetter setter)
{

}

ScriptEngine::ModuleRegister::ModuleRegister()
	: d(new Private(this)) {}
*/
}
