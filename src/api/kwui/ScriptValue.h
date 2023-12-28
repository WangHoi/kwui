#pragma once
#include "kwui_export.h"
#include <string>
#include <stdint.h>
#include <functional>

namespace kwui {

class ScriptValue;

typedef ScriptValue (ScriptFunction)(int argc, const ScriptValue* argv, void* udata);
//typedef ScriptValue (*ScriptPropertyGetter)(ScriptEngine* ctx, ScriptValue this_val);
//typedef ScriptValue (*ScriptPropertySetter)(ScriptEngine* ctx, ScriptValue this_val, ScriptValue val);

class KWUI_EXPORT ScriptValue {
public:
	ScriptValue();
	ScriptValue(bool v);
	ScriptValue(int v);
	ScriptValue(double v);
	ScriptValue(const char* s);
	ScriptValue(const std::string& s);
	ScriptValue(const ScriptValue& o);
	ScriptValue(ScriptValue&& o) noexcept;
	~ScriptValue();
	static ScriptValue newArray();
	static ScriptValue newObject();

	ScriptValue& operator=(const ScriptValue& o);
	ScriptValue& operator=(ScriptValue&& o) noexcept;

	const ScriptValue& operator[](int idx) const;
	ScriptValue& operator[](int idx);
	const ScriptValue& operator[](const char* key) const;
	ScriptValue& operator[](const char* key);

	bool isNull() const;
	bool isBool() const;
	bool isNumber() const;
	bool isString() const;
	bool isArray() const;
	bool isObject() const;

	bool toBool() const;
	double toDouble() const;
	int toInt() const;
	std::string toString() const;
	
	void visitArray(std::function<void(int, const ScriptValue&)>&& f) const;
	void visitObject(std::function<void(const std::string&, const ScriptValue&)>&& f) const;

private:
	class Private;
	Private* d;
};

}
