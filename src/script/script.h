#pragma once

#include "quickjs.h"
#include "quickjs-libc.h"
#include "cutils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <fstream>

#include "base/base.h"
#include "style/style.h"

namespace scene2d {
class Scene;
}

namespace script {

class Context;
class Runtime {
public:
	static Runtime* get() {
		static Runtime rt;
		return &rt;
	}
private:
	Runtime()
	{
		rt_ = JS_NewRuntime();
		JS_SetRuntimeOpaque(rt_, this);
	}
	~Runtime()
	{
		JS_FreeRuntime(rt_);
		rt_ = nullptr;
	}

	JSRuntime* rt_;
	friend class Context;
};

class Context {
public:
	Context()
		: Context(Runtime::get()) {}
	Context(Runtime* rt);
	~Context();

	inline JSContext* get() const
	{
		return ctx_;
	}

	void loadFile(const std::string& fname)
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
		int eval_type = JS_DetectModule(content.data(), content.size())
			? JS_EVAL_TYPE_MODULE
			: JS_EVAL_TYPE_GLOBAL;
		JSValue ret = JS_Eval(ctx_, content.data(), size, fname.c_str(), eval_type);
		if (JS_IsException(ret)) {
			js_std_dump_error(ctx_);
		}
		JS_FreeValue(ctx_, ret);
	}

	template<typename T>
	T parse(JSValue j)
	{
		return T();
	}

	template<>
	bool parse<bool>(JSValue j)
	{
		return JS_ToBool(ctx_, j) == TRUE;
	}

	template<>
	std::string parse<std::string>(JSValue j)
	{
		const char* cstr = JS_ToCString(ctx_, j);
		if (cstr) {
			std::string s(cstr);
			JS_FreeCString(ctx_, cstr);
			return s;
		} else {
			return std::string();
		}
	}

	template<>
	base::string_atom parse<base::string_atom>(JSValue j)
	{
		const char* s = JS_ToCString(ctx_, j);
		return s ? base::string_intern(s) : base::string_atom();
	}

	template<>
	style::StyleSpec parse<style::StyleSpec>(JSValue j)
	{
		style::StyleSpec v;
		if (!JS_IsObjectPlain(ctx_, j))
			return v;
		JSPropertyEnum* ptab = nullptr;
		uint32_t plen = 0;
		JS_GetOwnPropertyNames(ctx_, &ptab, &plen, j, JS_GPN_STRING_MASK | JS_GPN_SYMBOL_MASK | JS_GPN_ENUM_ONLY);
		for (uint32_t i = 0; i < plen; ++i) {
			JSValue jval = JS_GetProperty(ctx_, j, ptab[i].atom);
			if (JS_IsException(jval))
				continue;
			char prop_name_buf[32] = {};
			JS_AtomGetStr(ctx_, prop_name_buf, 31, ptab[i].atom);
			base::string_atom prop = base::string_intern(prop_name_buf);
			style::ValueSpec val = parse<style::ValueSpec>(jval);
			JS_FreeValue(ctx_, jval);
		}
		return v;
	}

	template<>
	style::ValueSpec parse<style::ValueSpec>(JSValue j)
	{
		style::ValueSpec v;
		if (JS_IsNumber(j)) {
			v.value = parse<style::Value>(j);
			v.type = style::ValueSpecType::Specified;
		} else if (JS_IsString(j)) {
			v.value = parse<style::Value>(j);
			if (v.value->unit != style::ValueUnit::Undefined) {
				v.type = style::ValueSpecType::Specified;
			}
			if (v.value->unit == style::ValueUnit::Keyword) {
				if (v.value->keyword_val == base::string_intern("initial")) {
					v.type = style::ValueSpecType::Initial;
				} else if (v.value->keyword_val == base::string_intern("inherit")) {
					v.type = style::ValueSpecType::Inherit;
				}
			}
		}
		return v;
	}

	template<>
	style::Value parse<style::Value>(JSValue j)
	{
		style::Value v;
		if (JS_IsNumber(j)) {
			double f64;
			JS_ToFloat64(ctx_, &f64, j);
			v.f32_val = (float)f64;
			v.unit = style::ValueUnit::Raw;
		} else if (JS_IsString(j)) {
			const char* s = JS_ToCString(ctx_, j);
			int ret;
			char dim[32] = {};
			ret = sscanf(s, "%f%32s", &v.f32_val, dim);
			if (ret == 1) {
				v.unit = style::ValueUnit::Raw;
			} else if (ret == 2) {
				v.keyword_val = base::string_intern(dim);
				if (strcmp(dim, "auto") == 0) {
					v.unit = style::ValueUnit::Auto;
				} else if (strcmp(dim, "px") == 0) {
					v.unit = style::ValueUnit::Pixel;
				} else if (strcmp(dim, "pt") == 0) {
					v.unit = style::ValueUnit::Point;
				} else if (strcmp(dim, "%") == 0) {
					v.unit = style::ValueUnit::Percent;
				} else {
					v.unit = style::ValueUnit::Keyword;
				}
			}
		}
		return v;
	}

	JSValue wrapScene(scene2d::Scene* scene);

private:
	void initSceneClass();

	JSContext* ctx_;
};

}
