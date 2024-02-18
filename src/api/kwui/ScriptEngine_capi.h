#pragma once
#include "kwui_export.h"
#include "ScriptValue_capi.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef void KWUI_EXPORT(*kwui_LogCallback)(const char* msg);
	typedef kwui_ScriptValue* KWUI_EXPORT(*kwui_ScriptFunction)(int argc, kwui_ScriptValue* argv[], void* udata);

	typedef struct kwui_ScriptEngine kwui_ScriptEngine;

	void KWUI_EXPORT kwui_ScriptEngine_addGlobalFunction(
		const char* name, kwui_ScriptFunction func, void* udata);
	void KWUI_EXPORT kwui_ScriptEngine_removeGlobalFfunction(const char* name);
	void KWUI_EXPORT kwui_ScriptEngine_loadFile(const char* path);
	kwui_ScriptValue* KWUI_EXPORT kwui_ScriptEngine_callGlobalFunction(
		const char* name, int argc, kwui_ScriptValue* argv[]);
	
	void KWUI_EXPORT kwui_ScriptEngine_postEvent0(
		const char* event);
	void KWUI_EXPORT kwui_ScriptEngine_postEvent1(
		const char* event, kwui_ScriptValue* arg);
	kwui_ScriptValue* KWUI_EXPORT kwui_ScriptEngine_sendEvent0(
		const char* event);
	kwui_ScriptValue* KWUI_EXPORT kwui_ScriptEngine_sendEvent1(
		const char* event, kwui_ScriptValue* arg);
	void KWUI_EXPORT kwui_ScriptEngine_addEventListener(
		const char* event, kwui_ScriptFunction func, void* udata);
	bool KWUI_EXPORT kwui_ScriptEngine_removeEventListener(
		const char* event, kwui_ScriptFunction func, void* udata);

#ifdef __cplusplus
}
#endif
