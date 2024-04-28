#pragma once
#include "kwui_export.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef KWUI_MAIN
#ifdef __ANDROID__
	int __attribute__((visibility("default"))) kwui_main(int, char* []);
#define KWUI_MAIN kwui_main
#else
#define KWUI_MAIN main
#endif
#endif

	typedef void (*kwui_LogCallback)(const char* msg);

	typedef struct kwui_Application kwui_Application;

	kwui_Application* KWUI_EXPORT kwui_Application_new(int argc, char* argv[]);
	void KWUI_EXPORT kwui_Application_delete(kwui_Application* app);
	void KWUI_EXPORT kwui_Application_setLogCallback(kwui_LogCallback callback);
	bool KWUI_EXPORT kwui_Application_scriptReloadEnabled();
	void KWUI_EXPORT kwui_Application_enableScriptReload(bool enable);
	//bool KWUI_EXPORT kwui_application_preloadResourceArchive(kwui_Application* app, int id);
	bool KWUI_EXPORT kwui_Application_isMainThread();
	void KWUI_EXPORT kwui_Application_runInMainThread(void (*task)(void* /*udata*/), void* udata);
	void KWUI_EXPORT kwui_Application_setResourceRootDir(kwui_Application* app, const char* dir);
	void KWUI_EXPORT kwui_Application_setResourceRootData(kwui_Application* app, const uint8_t* data, size_t size);
	int KWUI_EXPORT kwui_Application_exec(kwui_Application* app);
	void KWUI_EXPORT kwui_Application_quit();

#ifdef __cplusplus
}
#endif
