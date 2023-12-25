#include "ScriptEngine.h"
#include "script/script.h"
#include "absl/base/call_once.h"
#include "absl/functional/bind_front.h"
#include <vector>
#include <string>

namespace kwui {

struct FunctionDef {
	std::string name;
	ScriptFunction* func;

	FunctionDef(const char* n, ScriptFunction* f)
		: name(n), func(f) {}
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
		JS_SetOpaque(jobj, def->func);

		JS_SetPropertyStr(j, global, def->name.c_str(), jobj);
		
		JS_FreeValue(j, global);
	}
	
	static ScriptValue wrap(JSContext* ctx, JSValueConst c)
	{
		if (JS_IsBool(c)) {
			return JS_ToBool(ctx, c);
		} else if (JS_IsNumber(c)) {
			double f64 = 0.0;
			JS_ToFloat64(ctx, &f64, c);
			return ScriptValue(f64);
		} else if (JS_IsString(c)) {
			const char* s = nullptr;
			size_t len = 0;
			s = JS_ToCStringLen(ctx, &len, c);
			std::string str(s, len);
			JS_FreeCString(ctx, s);
			return ScriptValue(str);
		} else if (JS_IsObject(c)) {
			auto port = (script::EventPort*)JS_GetOpaque2(ctx, c, script::EventPort::JS_CLASS_ID);
			if (port)
				return ScriptValue(port->id());
		}
		return ScriptValue();
	}

	static JSValue unwrap(JSContext* ctx, const ScriptValue& c)
	{
		if (c.isBool()) {
			return JS_NewBool(ctx, c.toBool());
		} else if (c.isNumber()) {
			return JS_NewFloat64(ctx, c.toDouble());
		} else if (c.isString()) {
			std::string s = c.toString();
			return JS_NewStringLen(ctx, s.data(), s.length());
		} else {
			return JS_NULL;
		}
	}

	static JSValue callScriptFunc(
		JSContext* ctx, JSValueConst func_obj,
		JSValueConst this_val, int argc, JSValueConst* argv,
		int flags)
	{
		ScriptFunction* func = (ScriptFunction*)JS_GetOpaque2(ctx, func_obj, script_func_clsid);

		std::vector<ScriptValue> args;
		args.reserve(argc + 1);

		for (int i = 0; i < argc; ++i) {
			args.emplace_back(wrap(ctx, argv[i]));
		}
		args.emplace_back();

		ScriptValue ret = func(argc, args.data());

		return unwrap(ctx, ret);
	}
	static JSClassID script_func_clsid;
	static JSClassDef script_func_class;

	ScriptEngine* q = nullptr;
	std::unique_ptr<script::Context> default_ctx;
	std::vector<std::unique_ptr<FunctionDef>> global_funcs;
	//std::vector<std::unique_ptr<ModuleRegister>> modules;
};

JSClassID ScriptEngine::Private::script_func_clsid = {};
JSClassDef ScriptEngine::Private::script_func_class = {
	"ScriptFunction",
	nullptr, // finalizer
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

void ScriptEngine::addGlobalFunction(const char* name, ScriptFunction* func)
{
	d->global_funcs.emplace_back(std::make_unique<FunctionDef>(name, func));
	script::Runtime::get()->eachContext(absl::bind_front(
		&Private::setGlobalFunction, d, d->global_funcs.back().get()));
}

void ScriptEngine::loadFile(const char* path)
{
	d->default_ctx->loadFile(path);
}

ScriptValue ScriptEngine::callGlobalFunction(const char* name, int argc, ScriptValue* argv)
{
	JSContext* ctx = d->default_ctx->get();
	JSValue global = JS_GetGlobalObject(ctx);
	JSValue func = JS_GetPropertyStr(ctx, global, name);
	ScriptValue ret;
	if (JS_IsFunction(ctx, func)) {
		std::vector<JSValue> jargs;
		for (int i = 0; i < argc; ++i) {
			jargs.push_back(Private::unwrap(ctx, argv[i]));
		}
		JSValue jret = JS_Call(ctx, func, JS_UNDEFINED, argc, jargs.data());
		ret = Private::wrap(ctx, jret);
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

bool ScriptEngine::postEvent(int port, const ScriptValue& value)
{
	return script::EventPort::postEvent(port, value);
}

bool ScriptEngine::addEventListener(int port_id, ScriptFunction* func)
{
	script::EventPort* port = script::EventPort::findEventPort(port_id);
	if (!port)
		return false;
	return port->addListener(d->script_func_clsid, func);
}

bool ScriptEngine::removeEventListener(int port_id, ScriptFunction* func)
{
	script::EventPort* port = script::EventPort::findEventPort(port_id);
	if (!port)
		return true;
	return port->removeListener(d->script_func_clsid, func);
}

int ScriptEngine::createEventPort()
{
	JSContext* ctx = d->default_ctx->get();
	JSValue obj = JS_NewObjectClass(ctx, script::EventPort::JS_CLASS_ID);
	auto port = (script::EventPort*)JS_GetOpaque2(ctx, obj, script::EventPort::JS_CLASS_ID);
	int port_id = port->id();
	JSValue global = JS_GetGlobalObject(ctx);
	std::string name = absl::StrFormat("__event_port_%d", port_id);
	JS_SetPropertyStr(ctx, global, name.c_str(), obj);
	JS_FreeValue(ctx, obj);
	JS_FreeValue(ctx, global);
	return port_id;
}

void ScriptEngine::releaseEventPort(int port)
{
	JSContext* ctx = d->default_ctx->get();
	JSValue global = JS_GetGlobalObject(ctx);
	std::string name = absl::StrFormat("__event_port_%d", port);
	JS_SetPropertyStr(ctx, global, name.c_str(), JS_UNDEFINED);
	JS_FreeValue(ctx, global);
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
