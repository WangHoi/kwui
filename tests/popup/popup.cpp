#include "kwui.h"
#include "kwui_capi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
using namespace kwui;

int main(int argc, char* argv[])
{
    Application app(argc, argv);
    ScriptEngine::get()
        ->loadFile("d:/projects/kwui/tests/popup/assets/entry.mjs");
    return app.exec();
}
