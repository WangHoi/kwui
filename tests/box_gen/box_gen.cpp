#include "kwui.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
using namespace kwui;

int main(int argc, char* argv[])
{
    Application app(argc, argv);
    ScriptEngine::get()
        ->loadFile("d:/projects/kwui/tests/box_gen/assets/box_gen.mjs");
    return app.exec();
}
