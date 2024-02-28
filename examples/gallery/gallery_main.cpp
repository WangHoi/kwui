#include "kwui.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
using namespace kwui;

int main(int argc, char* argv[])
{
    Application app(argc, argv);
    ScriptEngine::get()
        ->loadFile("d:/projects/kwui/examples/gallery/assets/gallery.js");
    return app.exec();
}
