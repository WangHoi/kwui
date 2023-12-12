#pragma once
#include <string>

namespace kwui {

class ScriptEngine;
class ScriptValue;

typedef ScriptValue (*ScriptFunction)(ScriptEngine* ctx, ScriptValue this_val, int argc, ScriptValue* argv);
typedef ScriptValue (*ScriptPropertyGetter)(ScriptEngine* ctx, ScriptValue this_val);
typedef ScriptValue (*ScriptPropertySetter)(ScriptEngine* ctx, ScriptValue this_val, ScriptValue val);

class ScriptValue {
public:
	bool isUndefined() const;
	bool isNull() const;
	bool isBool() const;
	bool isNumber() const;
	bool isString() const;
	bool isArray() const;
	bool isObject() const;

	bool toBool() const;
	float toFloat() const;
	int toInt() const;
	std::string toString() const;
	ScriptValue property(int index) const;
	ScriptValue property(const char* name) const;
};

}
