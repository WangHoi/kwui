#include "kwui.h"
#include <string.h>

#include "PlotElement.h"
using namespace kwui;

KWUI_MAIN()
{
    Application app(argc, argv);

#ifdef _WIN32
    const char* sep = strrchr(__FILE__, '\\');
    std::string dir(__FILE__, sep);
    app.setResourceRootDir((dir + "\\assets").c_str());
#endif

    app.registerCustomElement("plot", &PlotElementFactory);

    ScriptEngine::get()
        ->loadFile(":/entry.js");
    return app.exec();
}
