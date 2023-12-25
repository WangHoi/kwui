#include "kwui.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
using namespace kwui;

static int g_arg = 0;
static int g_port;

ScriptValue add(int argc, ScriptValue* argv)
{
    if (argc != 2) {
        return ScriptValue();
    }
    return argv[0].toDouble() + argv[1].toDouble();
}

ScriptValue onNativeEvent(int argc, ScriptValue* argv)
{
    if (argc != 2) {
        return ScriptValue();
    }
    g_arg = argv[0].toInt();
    g_port = argv[1].toInt();
    return true;
}

int main(int argc, char* argv[])
{
    Application app(argc, argv);
    auto SE = ScriptEngine::get();
    SE->addGlobalFunction("add", &add);
    SE->addGlobalFunction("onNativeEvent", &onNativeEvent);
    SE->loadFile("d:/projects/kwui/examples/native/assets/native.mjs");

    ScriptValue args[2] = { 3, 4 };
    ScriptValue ret = SE->callGlobalFunction("scriptAdd", 2, args);
    int val = ret.toInt();
    
    for (int i = 0; i < g_arg; ++i) {
        SE->postEvent(g_port, i + 1);
    }
    return app.exec();
}
