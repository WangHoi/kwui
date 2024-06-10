#pragma once
#include "kwui_export.h"
#include "ScriptValue_capi.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef void (*kwui_LogCallback)(const char* msg);
	typedef kwui_ScriptValue* (*kwui_ScriptFunction)(int argc, kwui_ScriptValue* argv[], void* udata);

	typedef struct kwui_ScriptEngine kwui_ScriptEngine;

	KWUI_EXPORT void kwui_ScriptEngine_addGlobalFunction(
		const char* name, kwui_ScriptFunction func, void* udata);
	KWUI_EXPORT void kwui_ScriptEngine_removeGlobalFunction(const char* name);
	KWUI_EXPORT void kwui_ScriptEngine_loadFile(const char* path);
	KWUI_EXPORT kwui_ScriptValue* kwui_ScriptEngine_callGlobalFunction(
		const char* name, int argc, kwui_ScriptValue* argv[]);
	
	KWUI_EXPORT void kwui_ScriptEngine_postEvent0(
		const char* event);
	KWUI_EXPORT void kwui_ScriptEngine_postEvent1(
		const char* event, kwui_ScriptValue* arg);
	KWUI_EXPORT kwui_ScriptValue* kwui_ScriptEngine_sendEvent0(
		const char* event);
	KWUI_EXPORT kwui_ScriptValue* kwui_ScriptEngine_sendEvent1(
		const char* event, kwui_ScriptValue* arg);
	KWUI_EXPORT void kwui_ScriptEngine_addEventListener(
		const char* event, kwui_ScriptFunction func, void* udata);
	KWUI_EXPORT void kwui_ScriptEngine_removeEventListener(
		const char* event, kwui_ScriptFunction func, void* udata);

#ifdef __cplusplus
}
#endif
