#pragma once
#include "kwui_export.h"

namespace kwui {

typedef void KWUI_EXPORT (*LogCallback)(const char* msg);

class KWUI_EXPORT Application {
public:
	Application(int argc, char* argv[]);
	~Application();
	static void setLogCallback(LogCallback callback);
	int exec();
};

}
