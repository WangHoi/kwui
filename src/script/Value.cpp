#include "Value.h"
#include "api/kwui/ScriptValue.h"
#include "base/log.h"

namespace script {
Value::Value()
	: ctx_(nullptr), value_(JS_UNDEFINED)
{
}

Value::Value(JSContext* ctx, const kwui::ScriptValue& v)
	: ctx_(ctx)
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
	: ctx_(ctx)
{
	if (ctx_) {
		value_ = JS_DupValue(ctx_, v);
	} else {
		value_ = v;
	}
}

Value::Value(const Value& o)
	: ctx_(o.ctx_)
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
	if (ctx_)
		JS_FreeValue(ctx_, value_);
}

Value& Value::operator=(const Value& o)
{
	if (this == &o)
		return *this;

	if (ctx_)
		JS_FreeValue(ctx_, value_);

	ctx_ = o.ctx_;
	if (ctx_) {
		value_ = JS_DupValue(ctx_, o.value_);
	} else {
		value_ = o.value_;
	}
	return *this;
}

Value& Value::operator=(Value&& o) noexcept
{
	if (this == &o)
		return *this;

	if (ctx_)
		JS_FreeValue(ctx_, value_);

	ctx_ = o.ctx_;
	value_ = o.value_;
	o.ctx_ = nullptr;
	o.value_ = JS_UNDEFINED;
	return *this;
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