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

class ScriptEngine::Private {
public:
	Private(ScriptEngine* se)
		: q(se)
	{
		script::Runtime::get()->addContextSetupFunc(absl::bind_front(&Private::initContext, this));
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
	

	static JSValue callScriptFunc(
		JSContext* ctx, JSValueConst func_obj,
		JSValueConst this_val, int argc, JSValueConst* argv,
		int flags)
	{
		ScriptFunction* func = (ScriptFunction*)JS_GetOpaque2(ctx, func_obj, script_func_clsid);

		func(argc, &ScriptValue(ctx, argv[0]));

		return JS_UNDEFINED;
	}
	static JSClassID script_func_clsid;
	static JSClassDef script_func_class;

	ScriptEngine* q = nullptr;
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
	static ScriptEngine* g_engine = nullptr;
	ABSL_CONST_INIT static absl::once_flag g_once;

	absl::call_once(g_once, [&]() {
		g_engine = new ScriptEngine;
		});

	return g_engine;
}

void ScriptEngine::addGlobalFunction(const char* name, ScriptFunction* func)
{
	d->global_funcs.emplace_back(std::make_unique<FunctionDef>(name, func));
	script::Runtime::get()->eachContext(absl::bind_front(
		&Private::setGlobalFunction, d, d->global_funcs.back().get()));
}

//ScriptEngine::ModuleRegister& ScriptEngine::addGlobalModule(const char* name)
//{
//	d->modules.emplace_back(std::make_unique<ModuleRegister>());
//	return *d->modules.back();
//}

ScriptEngine::ScriptEngine()
	: d(new Private(this))
{}
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
