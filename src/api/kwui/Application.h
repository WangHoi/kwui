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
	void setResourceRootData(const uint8_t* data, size_t len);
	void addFont(const char* family_name, const char* font_path);
	int exec();
	static void quit();

private:
	class Private;
	Private* d;
};

}
