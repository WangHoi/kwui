#pragma once

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
	void gc();

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

	inline JSContext* get() const { return ctx_; }

	void loadFile(const std::string& fname);

	template<typename T>
	T parse(JSValue j)
	{
		return T();
	}
	template<typename T>
	static T parse(JSContext *, JSValue)
	{
		return T();
	}

	template<>
	bool parse<bool>(JSValue j)
	{
		return Context::parse<bool>(ctx_, j);
	}
	template<>
	static bool parse<bool>(JSContext *ctx_, JSValue j)
	{
		return !!JS_ToBool(ctx_, j);
	}

	template<>
	std::string parse<std::string>(JSValue j)
	{
		return Context::parse<std::string>(ctx_, j);
	}
	template<>
	static std::string parse<std::string>(JSContext *ctx_, JSValue j)
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
		return Context::parse<base::string_atom>(ctx_, j);
	}
	template<>
	static base::string_atom parse<base::string_atom>(JSContext* ctx_, JSValue j)
	{
		const char* s = JS_ToCString(ctx_, j);
		return s ? base::string_intern(s) : base::string_atom();
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
		return Context::parse<style::StyleSpec>(ctx_, j);
	}

	template<>
	static style::StyleSpec parse<style::StyleSpec>(JSContext* ctx_, JSValue j);

	template<>
	style::ValueSpec parse<style::ValueSpec>(JSValue j)
	{
		return Context::parse<style::ValueSpec>(ctx_, j);
	}
	template<>
	static style::ValueSpec parse<style::ValueSpec>(JSContext* ctx_, JSValue j);

	template<>
	style::Value parse<style::Value>(JSValue j)
	{
		return Context::parse<style::Value>(ctx_, j);
	}
	template<>
	static style::Value parse<style::Value>(JSContext* ctx_, JSValue j);
	JSValue wrapScene(scene2d::Scene* scene);

private:
	void initSceneClass();

	JSContext* ctx_;
	JSValue app_ = JS_UNINITIALIZED;
};

template<>
inline style::StyleSpec Context::parse<style::StyleSpec>(JSContext* ctx_, JSValue j)
{
	style::StyleSpec v;
	Context::eachObjectField(ctx_, j, [&](const char* prop_name, JSValue jval) {
		auto prop = base::string_intern(prop_name);
		style::ValueSpec val = Context::parse<style::ValueSpec>(ctx_, jval);
#define CHECK_VALUE(x) if (prop == base::string_intern(#x)) { v.x = val; }
#define CHECK_VALUE2(x, name) if (prop == base::string_intern(name)) { v.x = val; }
		CHECK_VALUE(display);
		CHECK_VALUE(position);

		CHECK_VALUE2(margin_left, "margin-left");
		CHECK_VALUE2(margin_top, "margin-top");
		CHECK_VALUE2(margin_right, "margin-right");
		CHECK_VALUE2(margin_bottom, "margin-bottom");
		if (prop == base::string_intern("margin"))
			v.margin_left = v.margin_top = v.margin_right = v.margin_bottom = val;

		CHECK_VALUE2(border_left_width, "border-left-width");
		CHECK_VALUE2(border_top_width, "border-top-width");
		CHECK_VALUE2(border_right_width, "border-right-width");
		CHECK_VALUE2(border_bottom_width, "border-bottom-width");
		if (prop == base::string_intern("border-width"))
			v.border_left_width = v.border_top_width = v.border_right_width = v.border_bottom_width = val;

		CHECK_VALUE2(border_top_left_radius, "border-top-left-radius");
		CHECK_VALUE2(border_top_right_radius, "border-top-right-radius");
		CHECK_VALUE2(border_bottom_right_radius, "border-bottom-right-radius");
		CHECK_VALUE2(border_bottom_left_radius, "border-bottom-left-radius");
		if (prop == base::string_intern("border-radius"))
			v.border_top_left_radius = v.border_top_right_radius = v.border_bottom_right_radius = v.border_bottom_left_radius = val;

		CHECK_VALUE2(padding_left, "padding-left");
		CHECK_VALUE2(padding_top, "padding-top");
		CHECK_VALUE2(padding_right, "padding-right");
		CHECK_VALUE2(padding_bottom, "padding-bottom");
		if (prop == base::string_intern("padding"))
			v.padding_left = v.padding_top = v.padding_right = v.padding_bottom = val;

		CHECK_VALUE(left);
		CHECK_VALUE(top);
		CHECK_VALUE(right);
		CHECK_VALUE(bottom);

		CHECK_VALUE(width);
		CHECK_VALUE(height);

		CHECK_VALUE2(border_color, "border-color");
		CHECK_VALUE2(background_color, "background-color");
		CHECK_VALUE(color);

		CHECK_VALUE2(line_height, "line-height");
		CHECK_VALUE2(font_family, "font-family");
		CHECK_VALUE2(font_size, "font-size");
		CHECK_VALUE2(font_style, "font-style");
		CHECK_VALUE2(font_weight, "font-weight");
		CHECK_VALUE2(text_align, "text-align");
#undef CHECK_VALUE
#undef CHECK_VALUE2
		});
	return v;
}

template<>
inline style::ValueSpec Context::parse<style::ValueSpec>(JSContext* ctx_, JSValue j)
{
	style::ValueSpec v;
	if (JS_IsNumber(j)) {
		v.value = Context::parse<style::Value>(ctx_, j);
		v.type = style::ValueSpecType::Specified;
	} else if (JS_IsString(j)) {
		v.value = Context::parse<style::Value>(ctx_, j);
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
inline style::Value Context::parse<style::Value>(JSContext* ctx_, JSValue j)
{
	style::Value v;
	if (JS_IsNumber(j)) {
		double f64;
		JS_ToFloat64(ctx_, &f64, j);
		v.f32_val = (float)f64;
		v.unit = style::ValueUnit::Raw;
	} else if (JS_IsString(j)) {
		std::string s = Context::parse<std::string>(ctx_, j);
		if (absl::StartsWith(s, "#")) {
			v.string_val = s;
			v.unit = style::ValueUnit::HexColor;
		} else if (absl::StartsWith(s, "url(") && absl::EndsWith(s, ")")) {
			s = s.substr(4, s.length() - 5);
			if (s.length() >= 2 && absl::StartsWith(s, "\"") && absl::EndsWith(s, "\"")) {
				v.string_val = s.substr(1, s.length() - 2);
			} else {
				v.string_val = s;
			}
			v.unit = style::ValueUnit::Url;
		} else {
			int ret;
			float f32_val;
			char dim[32] = {};
			ret = sscanf(s.c_str(), "%f%32s", &f32_val, dim);
			if (ret == 1) {
				v.f32_val = f32_val;
				v.unit = style::ValueUnit::Raw;
			} else if (ret == 2) {
				if (strcmp(dim, "px") == 0) {
					v.f32_val = f32_val;
					v.unit = style::ValueUnit::Pixel;
				} else if (strcmp(dim, "pt") == 0) {
					v.f32_val = f32_val;
					v.unit = style::ValueUnit::Point;
				} else if (strcmp(dim, "%") == 0) {
					v.f32_val = f32_val;
					v.unit = style::ValueUnit::Percent;
				} else {
					v.keyword_val = base::string_intern(dim);
					v.unit = style::ValueUnit::Keyword;
				}
			} else {
				v.keyword_val = base::string_intern(s);
				v.unit = style::ValueUnit::Keyword;
			}
		}
	}
	return v;
}

}
