#pragma once
#include "kwui_export.h"
#include "ScriptValue.h"

namespace kwui {

class KWUI_EXPORT ScriptEngine {
public:
	//class ModuleRegister;

	static ScriptEngine* get();
	static void release();
	void addGlobalFunction(const char* name, ScriptFunction* func);
	void loadFile(const char* path);
	//ModuleRegister& addGlobalModule(const char* name);

private:
	ScriptEngine();
	~ScriptEngine();
	ScriptEngine(const ScriptEngine&) = delete;
	ScriptEngine& operator=(const ScriptEngine&) = delete;

	class Private;
	Private* d;
};
/*
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
*/
}