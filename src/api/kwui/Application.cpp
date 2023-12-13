#include "Application.h"
#include "ScriptEngine.h"
#include "windows/graphics/GraphicDevice.h"
#include "windows/Dialog.h"
#include "windows/control/Control.h"
#include "windows/control/ButtonControl.h"
#include "windows/control/ImageControl.h"
#include "windows/control/ImageButtonControl.h"
#include "windows/control/LineEditControl.h"
#include "windows/control/ProgressBarControl.h"
#include "windows/ResourceManager.h"
#include <ConsoleApi2.h>

namespace kwui {

Application::Application(int argc, char* argv[])
{
    ::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    base::initialize_log();
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    ::SetConsoleOutputCP(65001);

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
}

Application::~Application()
{
    ScriptEngine::release();
    windows::graphics::GraphicDevice::releaseInstance();
    windows::ResourceManager::releaseInstance();

    CoUninitialize();
}

int Application::exec()
{
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

}
