#pragma once
#include "quickjs.h"

namespace kwui {
class ScriptValue;
}

namespace script {

class Value {
public:
	Value(); // default 'undefined'
	explicit Value(JSContext* ctx, const kwui::ScriptValue& v);
	explicit Value(JSContext* ctx, JSValue v);
	Value(const Value& o);
	Value(Value&& o) noexcept;
	~Value();

	Value& operator=(const Value& o);
	Value& operator=(Value&& o) noexcept;

	bool isUndefined() const;
	bool operator==(const Value& o);
	bool operator==(JSValue o);

	inline JSContext* jsContext() const
	{
		return ctx_;
	}
	inline const JSValue& jsValue() const
	{
		return value_;
	}
	inline JSValue& jsValue()
	{
		return value_;
	}

private:
	JSContext* ctx_;
	JSValue value_;
};

}