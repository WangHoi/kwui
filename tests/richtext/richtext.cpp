#include "kwui.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
using namespace kwui;

KWUI_MAIN()
{
    Application app(argc, argv);
#ifdef _WIN32
    app.setResourceRootDir("d:/projects/kwui/tests/richtext/assets");
#endif
    ScriptEngine::get()
        ->loadFile(":/entry.js");
    return app.exec();
}
