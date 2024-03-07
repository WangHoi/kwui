#include "Application_capi.h"
#include "Application.h"

kwui_Application* kwui_Application_new(int argc, char* argv[])
{
	return (kwui_Application*)(new kwui::Application(argc, argv));
}

void kwui_Application_delete(kwui_Application* app)
{
	delete (kwui::Application*)(app);
}

void kwui_Application_setLogCallback(kwui_LogCallback callback)
{
	kwui::Application::setLogCallback(callback);
}

bool kwui_Application_scriptReloadEnabled()
{
	return kwui::Application::scriptReloadEnabled();
}

void kwui_Application_enableScriptReload(bool enable)
{
	kwui::Application::enableScriptReload(enable);
}

bool kwui_Application_isMainThread()
{
	return kwui::Application::isMainThread();
}
void kwui_Application_runInMainThread(void (*task)(void* /*udata*/), void* udata)
{
	if (task) {
		kwui::Application::runInMainThread([=]() { task(udata); });
	}
}

//bool kwui_application_preload_resource_archive(kwui_Application* app, int id);
void kwui_Application_setResourceRootDir(kwui_Application* app, const char* dir)
{
	return ((kwui::Application*)app)->setResourceRootDir(dir);
}

int kwui_Application_exec(kwui_Application* app)
{
	return ((kwui::Application*)app)->exec();
}

void kwui_Application_quit()
{
	kwui::Application::quit();
}
