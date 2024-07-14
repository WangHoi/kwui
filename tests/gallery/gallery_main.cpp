#include "kwui.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
using namespace kwui;

int main(int argc, char* argv[])
{
    Application app(argc, argv);
#if defined(_DEBUG) && !defined(__ANDROID__)
    app.setResourceRootDir("d:/projects/kwui-rs/kwui-sys/deps/kwui/tests/gallery/assets");
#endif
    ScriptEngine::get()->loadFile(":/gallery.js");
    return app.exec();
}
