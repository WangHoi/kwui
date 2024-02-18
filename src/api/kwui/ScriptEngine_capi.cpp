#include "ScriptEngine_capi.h"
#include "ScriptEngine.h"

void KWUI_EXPORT kwui_ScriptEngine_addGlobalFunction(const char* name, kwui_ScriptFunction func, void* udata)
{
	return;
}

void KWUI_EXPORT kwui_ScriptEngine_removeGlobalFfunction(const char* name)
{
	return;
}

void KWUI_EXPORT kwui_ScriptEngine_loadFile(const char* path)
{
	kwui::ScriptEngine::get()
		->loadFile(path);
}

kwui_ScriptValue* KWUI_EXPORT kwui_ScriptEngine_callGlobalFunction(const char* name, int argc, kwui_ScriptValue* argv[])
{
	return nullptr;
}

void KWUI_EXPORT kwui_ScriptEngine_postEvent0(const char* event)
{
	return;
}

void KWUI_EXPORT kwui_ScriptEngine_postEvent1(const char* event, kwui_ScriptValue* arg)
{
	return;
}

kwui_ScriptValue* KWUI_EXPORT kwui_ScriptEngine_sendEvent0(const char* event)
{
	return nullptr;
}

kwui_ScriptValue* KWUI_EXPORT kwui_ScriptEngine_sendEvent1(const char* event, kwui_ScriptValue* arg)
{
	return nullptr;
}

void KWUI_EXPORT kwui_ScriptEngine_addEventListener(const char* event, kwui_ScriptFunction func, void* udata)
{
	return;
}
