#pragma once
#include "kwui_export.h"

namespace kwui {

typedef void KWUI_EXPORT (*LogCallback)(const char* msg);

class KWUI_EXPORT Application {
public:
	Application(int argc, char* argv[]);
	Application(int argc, wchar_t* argv[]);
	~Application();
	static void setLogCallback(LogCallback callback);
	bool preloadResourceArchive(int id);
	int exec();

private:
	class Private;
	Private* d;
};

}
