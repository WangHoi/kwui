#pragma once

namespace kwui {

class Application {
public:
	Application(int argc, char* argv[]);
	~Application();
	int exec();
};

}
