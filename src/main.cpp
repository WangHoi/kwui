#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "scene2d/scene2d.h"
#include "script/script.h"
#include "windows/graphics/GraphicDevice.h"
#include "windows/Dialog.h"
#include "windows/control/Control.h"
#include "windows/control/ButtonControl.h"
#include "windows/control/ImageControl.h"
#include "windows/control/ImageButtonControl.h"
#include "windows/control/LineEditControl.h"
#include "windows/control/ProgressBarControl.h"
#include "windows/ResourceManager.h"

int main()
{
    ::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    base::initialize_log();
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    int exit_code = 0;
    MSG msg;
    PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE);

    windows::ResourceManager::createInstance(::GetModuleHandleW(NULL));
    windows::graphics::GraphicDevice::createInstance()->Init();

    windows::graphics::GraphicDevice::instance()
        ->LoadBitmapToCache("cx_logo_2.svg", L"D:\\projects\\kwui\\chengxun80.png");

    scene2d::ControlRegistry::get()->registerControl<windows::control::LineEditControl>();
    scene2d::ControlRegistry::get()->registerControl<windows::control::ProgressBarControl>();
    scene2d::ControlRegistry::get()->registerControl<windows::control::ImageControl>();
    scene2d::ControlRegistry::get()->registerControl<windows::control::ButtonControl>();
    scene2d::ControlRegistry::get()->registerControl<windows::control::ImageButtonControl>();

    script::Context ctx;
    ctx.loadFile("d:/projects/kwui/button_layout.js");

    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    windows::graphics::GraphicDevice::releaseInstance();
    windows::ResourceManager::releaseInstance();

    CoUninitialize();
    return exit_code;
}
