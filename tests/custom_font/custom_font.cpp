#include "kwui.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
using namespace kwui;

int main(int argc, char* argv[])
{
    Application app(argc, argv);
    app.setResourceRootDir("d:/projects/kwui/tests/custom_font/assets");
    app.addFont("Font Awesome", ":/Font Awesome 6 Free-Regular-400.otf");
    ScriptEngine::get()
        ->loadFile(":/entry.js");
    return app.exec();
}
