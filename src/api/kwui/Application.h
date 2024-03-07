#pragma once
#include "kwui_export.h"
#include <functional>

namespace kwui {

typedef void KWUI_EXPORT (*LogCallback)(const char* msg);

class KWUI_EXPORT Application {
public:
	Application(int argc, char* argv[]);
	Application(int argc, wchar_t* argv[]);
	~Application();
	static void setLogCallback(LogCallback callback);
	static bool scriptReloadEnabled();
	static void enableScriptReload(bool enable);
	static bool isMainThread();
	static void runInMainThread(std::function<void()>&& func);
	bool preloadResourceArchive(int id);
	void setResourceRootDir(const char* dir);
	int exec();
	static void quit();

private:
	class Private;
	Private* d;
};

}
