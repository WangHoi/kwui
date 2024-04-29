// Write C++ code here.
//
// Do not forget to dynamically load the C++ library into your application.
//
// For instance,
//
// In MainActivity.java:
//    static {
//       System.loadLibrary("myapplication");
//    }
//
// Or, in MainActivity.kt:
//    companion object {
//      init {
//         System.loadLibrary("myapplication")
//      }
//    }
#include <kwui.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
using namespace kwui;

extern "C" int KWUI_MAIN(int argc, char* argv[])
{
    Application app(argc, argv);
    //app.setResourceRootDir("d:/projects/kwui/tests/richtext/assets");
    ScriptEngine::get()->loadFile(":/entry.js");
    return app.exec();
}