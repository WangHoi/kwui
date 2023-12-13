#pragma once
#include "kwui_export.h"

namespace kwui {

class KWUI_EXPORT Application {
public:
	Application(int argc, char* argv[]);
	~Application();
	int exec();
};

}
