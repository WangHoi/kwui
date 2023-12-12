#pragma once
#include "ScriptValue.h"

namespace kwui {

class ScriptEngine {
public:
	class ModuleRegister;

	static ScriptEngine* get();
	template <ScriptValue(*Func)(int argc, ScriptValue* argv)>
	void addGlobalFunction(const char* name, int min_args, Func func);
	ModuleRegister& addGlobalModule(const char* name);

private:
	ScriptEngine();
	ScriptEngine(const ScriptEngine&) = delete;
	ScriptEngine& operator=(const ScriptEngine&) = delete;

	class Private;
	Private* d;
};

class ScriptEngine::ModuleRegister {
public:
	void addFunction(const char* name, int min_args, ScriptFunction func);
	void addProperty(const char* name, ScriptPropertyGetter getter, ScriptPropertySetter setter = nullptr);

private:
	ModuleRegister();
	ModuleRegister(const ModuleRegister&) = delete;
	ModuleRegister& operator=(const ModuleRegister&) = delete;

	class Private;
	Private* d;
};

}
