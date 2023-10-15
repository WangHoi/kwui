#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "scene2d/scene2d.h"
#include "script/script.h"
#include "windows/graphics/GraphicDevice.h"
#include "windows/Dialog.h"
#include "windows/control/LineEditControl.h"
#include "windows/control/ProgressBarControl.h"

int main()
{
    ::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    base::initialize_log();
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    int exit_code = 0;
    MSG msg;
    PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE);

    windows::graphics::GraphicDevice::get()->Init();
    windows::control::register_line_edit_control();
    windows::control::register_progress_bar_control();

    script::Context ctx;
    ctx.loadFile("d:/projects/kwui/test.js");

    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CoUninitialize();
    return exit_code;
}