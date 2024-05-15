#pragma once

#include "ContextId.h"
#include "Value.h"
#include "ComponentState.h"
#include "EventPort.h"
#include "ElementRef.h"

#include "quickjs.h"
#include "quickjs-libc.h"
//#include "cutils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <fstream>
#include <vector>

#include "base/base.h"
#include "style/style.h"

#include "absl/strings/match.h"
#include "absl/status/statusor.h"
#include "absl/functional/function_ref.h"

namespace scene2d {
class Scene;
}

namespace script {

class Context;
class Runtime {
public:
	static Runtime* createInstance();
	static void releaseInstance();
	static Runtime* get();
	void gc();
	JSMemoryUsage computeMemoryUsage();
	void addContextSetupFunc(std::function<void(Context*)>&& new_ctx_func);
	void eachContext(absl::FunctionRef<void(Context*)> func);

private:
	Runtime();
	~Runtime();

	JSRuntime* rt_;
	std::vector<Context*> contexts_;
	std::vector<std::function<void(Context*)>> new_ctx_funcs_;
	friend class Context;
};

template<typename T>
T parse(JSContext *, JSValue)
{
	return T();
}
template<> bool parse<bool>(JSContext *ctx_, JSValue j);
template<> std::string parse<std::string>(JSContext *ctx_, JSValue j);
template<> base::string_atom parse<base::string_atom>(JSContext* ctx_, JSValue j);
template<> style::StyleSpec parse<style::StyleSpec>(JSContext* ctx_, JSValue j);
template<> style::ValueSpec parse<style::ValueSpec>(JSContext* ctx_, JSValue j);
template<> style::Value parse<style::Value>(JSContext* ctx_, JSValue j);

class Context {
public:
	Context()
		: Context(Runtime::get()) {}
	Context(Runtime* rt);
	~Context();

	inline JSContext* get() const { return ctx_; }

	void loadFile(const std::string& fname);
	void loadScript(const std::string& fname, const std::string& content);
	size_t reloadVersion() const { return reload_version_; }
	void incrementReloadVersion() { ++reload_version_; }

	template<typename T>
	T parse(JSValue j)
	{
		return T();
	}
	// template<typename T>
	// static T parse(JSContext *, JSValue)
	// {
	// 	return T();
	// }

	template<>
	bool parse<bool>(JSValue j)
	{
		return ::script::parse<bool>(ctx_, j);
	}
	template<>
	std::string parse<std::string>(JSValue j)
	{
		return ::script::parse<std::string>(ctx_, j);
	}
	template<>
	base::string_atom parse<base::string_atom>(JSValue j)
	{
		return ::script::parse<base::string_atom>(ctx_, j);
	}
	template<typename F>
	void eachObjectField(JSValue j, F&& f)
	{
		Context::eachObjectField(ctx_, j, std::move(f));
	}

	template<typename F>
	static void eachObjectField(JSContext* ctx, JSValue j, F&& f)
	{
		JSPropertyEnum* ptab = nullptr;
		uint32_t plen = 0;
		JS_GetOwnPropertyNames(ctx, &ptab, &plen, j, JS_GPN_STRING_MASK | JS_GPN_ENUM_ONLY);
		for (uint32_t i = 0; i < plen; ++i) {
			JSValue jval = JS_GetProperty(ctx, j, ptab[i].atom);
			if (JS_IsException(jval))
				continue;
			char prop_name_buf[32] = {};
			const char* prop_name = JS_AtomGetStr(ctx, prop_name_buf, 31, ptab[i].atom);
			f(prop_name, jval);
			JS_FreeValue(ctx, jval);
		}
		if (ptab)
			js_free_prop_enum(ctx, ptab, plen);
	}

	template<>
	style::StyleSpec parse<style::StyleSpec>(JSValue j)
	{
		return ::script::parse<style::StyleSpec>(ctx_, j);
	}

    template<>
	style::ValueSpec parse<style::ValueSpec>(JSValue j)
	{
		return ::script::parse<style::ValueSpec>(ctx_, j);
	}

	template<>
	style::Value parse<style::Value>(JSValue j)
	{
		return ::script::parse<style::Value>(ctx_, j);
	}

private:
	JSContext* ctx_;
	JSValue app_ = JS_UNINITIALIZED;
	size_t reload_version_ = 0;
};

static inline JSCFunctionListEntry js_cfunc_def(const char* name, uint8_t minargs, JSCFunction* pf)
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

static inline JSCFunctionListEntry js_cgetset_def(
	const char* name,
	JSValue (*fgetter)(JSContext*, JSValueConst),
	JSValue (*fsetter)(JSContext*, JSValueConst, JSValueConst) = nullptr)
{
	// #define JS_CGETSET_DEF(name, fgetter, fsetter) { name, JS_PROP_CONFIGURABLE, JS_DEF_CGETSET, 0,
	//		.u = { .getset = { .get = { .getter = fgetter }, .set = { .setter = fsetter } } } }
	JSCFunctionListEntry def = { 0 };
	def.name = name;
	def.prop_flags = JS_PROP_CONFIGURABLE;
	def.def_type = JS_DEF_CGETSET;
	def.magic = 0;
	def.u.getset.get.getter = fgetter;
	def.u.getset.set.setter = fsetter;
	return def;
}

kwui::ScriptValue wrap(JSContext* ctx, JSValueConst c);
JSValue unwrap(JSContext* ctx, const kwui::ScriptValue& c);

}
