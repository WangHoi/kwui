#include "kwui.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
using namespace kwui;

KWUI_MAIN()
{
    Application app(argc, argv);
    ScriptEngine::get()
        ->loadFile(":/box_gen.js");
    return app.exec();
}
