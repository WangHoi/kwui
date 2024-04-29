#include "kwui.h"
#include "kwui_capi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
using namespace kwui;

int KWUI_MAIN(int argc, char* argv[])
{
    //Application app(argc, argv);
    //ScriptEngine::get()
    //    ->loadFile("d:/projects/kwui/tests/overflow/assets/entry.mjs");
    //return app.exec();
    auto app = kwui_Application_new(argc, argv);
    kwui_ScriptEngine_loadFile("d:/projects/kwui/tests/overflow/assets/entry.js");
    return kwui_Application_exec(app);
}
