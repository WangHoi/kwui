#pragma once
#include "kwui_export.h"
#include <string>
#include <stdint.h>

namespace kwui {

class ScriptValue;

typedef ScriptValue (ScriptFunction)(int argc, ScriptValue* argv);
//typedef ScriptValue (*ScriptPropertyGetter)(ScriptEngine* ctx, ScriptValue this_val);
//typedef ScriptValue (*ScriptPropertySetter)(ScriptEngine* ctx, ScriptValue this_val, ScriptValue val);

class KWUI_EXPORT ScriptValue {
public:
	ScriptValue();
	ScriptValue(bool v);
	ScriptValue(int v);
	ScriptValue(double v);
	ScriptValue(const std::string& s);
	~ScriptValue();
	bool isNull() const;
	bool isBool() const;
	bool isNumber() const;
	bool isString() const;
	//bool isArray() const;
	//bool isObject() const;

	bool toBool() const;
	double toDouble() const;
	int toInt() const;
	std::string toString() const;
	//ScriptValue property(int index) const;
	//ScriptValue property(const char* name) const;

private:
	class Private;
	Private* d;
};

}
