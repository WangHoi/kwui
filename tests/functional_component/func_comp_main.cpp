#include "kwui.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
using namespace kwui;

int main(int argc, char* argv[])
{
    Application app(argc, argv);
    auto SE = ScriptEngine::get();
    SE->loadFile(":/functional.js");

    return app.exec();
}
