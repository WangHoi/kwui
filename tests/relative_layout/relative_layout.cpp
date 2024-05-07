#include "kwui.h"
#include "kwui_capi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
using namespace kwui;

KWUI_MAIN()
{
    auto app = kwui_Application_new(argc, argv);
#ifdef _WIN32
    kwui_Application_setResourceRootDir(app, "d:/projects/kwui-rs/kwui-sys/deps/kwui/tests/relative_layout/assets");
#endif
    kwui_ScriptEngine_loadFile(":/js/entry.js");
    return kwui_Application_exec(app);
}
