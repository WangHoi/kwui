#include "kwui.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
using namespace kwui;

int main(int argc, char* argv[])
{
    Application app(argc, argv);
    app.setResourceRootDir("d:/projects/kwui/tests/richtext/assets");
    ScriptEngine::get()
        ->loadFile("d:/projects/kwui/tests/richtext/assets/entry.js");
    return app.exec();
}
