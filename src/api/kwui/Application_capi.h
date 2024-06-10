#pragma once
#include "kwui_export.h"
#include "kwui_main.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef void (*kwui_LogCallback)(const char* msg);

	typedef struct kwui_Application kwui_Application;

	KWUI_EXPORT kwui_Application* kwui_Application_new(int argc, char* argv[]);
	KWUI_EXPORT void kwui_Application_delete(kwui_Application* app);
	KWUI_EXPORT void kwui_Application_setLogCallback(kwui_LogCallback callback);
	KWUI_EXPORT bool kwui_Application_scriptReloadEnabled();
	KWUI_EXPORT void kwui_Application_enableScriptReload(bool enable);
	//KWUI_EXPORT bool kwui_application_preloadResourceArchive(kwui_Application* app, int id);
	KWUI_EXPORT bool kwui_Application_isMainThread();
	KWUI_EXPORT void kwui_Application_runInMainThread(void (*task)(void* /*udata*/), void* udata);
	KWUI_EXPORT void kwui_Application_setResourceRootDir(kwui_Application* app, const char* dir);
	KWUI_EXPORT void kwui_Application_setResourceRootData(kwui_Application* app, const uint8_t* data, size_t size);
	KWUI_EXPORT int kwui_Application_exec(kwui_Application* app);
	KWUI_EXPORT void kwui_Application_quit();

#ifdef __cplusplus
}
#endif
