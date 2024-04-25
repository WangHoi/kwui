#include "Value.h"
#include "api/kwui/ScriptValue.h"
#include "base/log.h"

namespace script {
Value::Value()
	: ctx_(nullptr), value_(JS_UNDEFINED)
{
}

Value::Value(JSContext* ctx, const kwui::ScriptValue& v)
	: ctx_(ctx ? JS_DupContext(ctx) : nullptr)
{
	if (v.isNull()) {
		value_ = JS_NULL;
	} else if (v.isBool()) {
		value_ = v.toBool() ? JS_TRUE : JS_FALSE;
	} else if (v.isNumber()) {
		value_ = JS_NewFloat64(ctx, v.toDouble());
	} else if (v.isString()) {
		std::string s = v.toString();
		value_ = JS_NewStringLen(ctx, s.c_str(), s.size());
	} else {
		LOG(ERROR) << "unhandled ScriptValue type";
		value_ = JS_UNDEFINED;
	}
}

Value::Value(JSContext* ctx, JSValue v)
	: ctx_(ctx ? JS_DupContext(ctx) : nullptr)
{
	if (ctx_) {
		value_ = JS_DupValue(ctx_, v);
	} else {
		value_ = v;
	}
}

Value::Value(const Value& o)
	: ctx_(o.ctx_ ? JS_DupContext(o.ctx_) : nullptr)
{
	if (ctx_) {
		value_ = JS_DupValue(ctx_, o.value_);
	} else {
		value_ = o.value_;
	}
}

Value::Value(Value&& o) noexcept
	: ctx_(o.ctx_), value_(o.value_)
{
	o.ctx_ = nullptr;
	o.value_ = JS_UNDEFINED;
}

Value::~Value()
{
	if (ctx_) {
		JS_FreeValue(ctx_, value_);
		JS_FreeContext(ctx_);
	}
}

Value& Value::operator=(const Value& o)
{
	if (this == &o)
		return *this;

	auto old_ctx = ctx_;
	auto old_value = value_;

	ctx_ = o.ctx_ ? JS_DupContext(o.ctx_) : nullptr;
	if (ctx_) {
		value_ = JS_DupValue(ctx_, o.value_);
	} else {
		value_ = o.value_;
	}

	if (old_ctx) {
		JS_FreeValue(old_ctx, old_value);
		JS_FreeContext(old_ctx);
	}
	return *this;
}

Value& Value::operator=(Value&& o) noexcept
{
	if (this == &o)
		return *this;

	auto old_ctx = ctx_;
	auto old_value = value_;

	if (o.ctx_) {
		ctx_ = o.ctx_;
		value_ = JS_DupValue(ctx_, o.value_);
		JS_FreeValue(ctx_, o.value_);
	} else {
		ctx_ = o.ctx_;
		value_ = o.value_;
	}
	o.ctx_ = nullptr;
	o.value_ = JS_UNDEFINED;
	
	if (old_ctx) {
		JS_FreeValue(old_ctx, old_value);
		JS_FreeContext(old_ctx);
	}
	return *this;
}

bool Value::isUndefined() const
{
	return JS_IsUndefined(value_);
}

bool Value::isFunction() const
{
	return ctx_ && JS_IsFunction(ctx_, value_);
}

bool Value::operator==(const Value& o)
{
	return value_ == o.value_;
}

bool Value::operator==(JSValue o)
{
	return value_ == o;
}

}