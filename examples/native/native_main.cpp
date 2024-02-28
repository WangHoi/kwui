#include "kwui.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
using namespace kwui;

ScriptValue add(int argc, const ScriptValue* argv[], void*)
{
    if (argc != 2) {
        return ScriptValue();
    }
    return argv[0]->toDouble() + argv[1]->toDouble();
}

ScriptValue onTestEvent(int argc, const ScriptValue* argv[], void*)
{
    if (argc != 2) {
        return ScriptValue();
    }
    std::cout << "native recv: "
        << "event-name: " << argv[0]->toString()
        << ", arg: " << argv[1]->toString()
        << std::endl;
    return true;
}

int main(int argc, char* argv[])
{
    Application app(argc, argv);
    auto SE = ScriptEngine::get();
    SE->addGlobalFunction("add", &add, nullptr);
    SE->addEventListener("test-event", &onTestEvent, nullptr);
    SE->loadFile("d:/projects/kwui/examples/native/assets/native.js");

    ScriptValue a1 = 3, a2 = 4;
    const ScriptValue* args[2] = { &a1, &a2 };
    ScriptValue ret = SE->callGlobalFunction("scriptAdd", 2, args);
    int val = ret.toInt();
    
    std::cout << "native post event" << std::endl;
    SE->postEvent("test-event", "native-arg");

    return app.exec();
}
