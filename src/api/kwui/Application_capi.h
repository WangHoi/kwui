#pragma once
#include "kwui_export.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef void KWUI_EXPORT(*kwui_LogCallback)(const char* msg);

	typedef struct kwui_Application kwui_Application;

	kwui_Application* KWUI_EXPORT kwui_Application_new(int argc, char* argv[]);
	void KWUI_EXPORT kwui_Application_delete(kwui_Application* app);
	void KWUI_EXPORT kwui_Application_setLogCallback(kwui_LogCallback callback);
	//bool KWUI_EXPORT kwui_application_preloadResourceArchive(kwui_Application* app, int id);
	int KWUI_EXPORT kwui_Application_exec(kwui_Application* app);

#ifdef __cplusplus
}
#endif
