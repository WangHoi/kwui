#include "Application.h"
#include "ScriptEngine.h"
#include "scene2d/KmlControl.h"
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

static LogCallback g_log_callback = nullptr;
#ifdef NDEBUG
static bool g_script_reload_enabled = false;
#else
static bool g_script_reload_enabled = true;
#endif

class Application::Private {
public:
    Private(Application* q_)
        : q(q_)
    {}
    void init()
    {
        //::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        base::initialize_log(g_log_callback);
        CoInitializeEx(NULL, COINIT_MULTITHREADED);

        ::SetConsoleOutputCP(65001);

        MSG msg;
        PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE);

        LOG(INFO) << "Init ResourceManager...";
        windows::ResourceManager::createInstance(::GetModuleHandleW(NULL));
        LOG(INFO) << "Init GraphicDevice...";
        windows::graphics::GraphicDevice::createInstance()->Init();

        //windows::graphics::GraphicDevice::instance()
        //    ->LoadBitmapToCache("cx_logo_2.svg", L"D:\\projects\\kwui\\chengxun80.png");
        //windows::graphics::GraphicDevice::instance()
        //    ->LoadBitmapToCache("close_button.png", L"D:\\projects\\kwui\\close_button.png");
        //windows::graphics::GraphicDevice::instance()
        //    ->LoadBitmapToCache("close_button_hover.png", L"D:\\projects\\kwui\\close_button_hover.png");
        windows::graphics::GraphicDevice::instance()
            ->LoadBitmapToCache("expand.png", L"D:\\Projects\\kwui\\expand.png");
        windows::graphics::GraphicDevice::instance()
            ->LoadBitmapToCache("collapse.png", L"D:\\Projects\\kwui\\collapse.png");

        LOG(INFO) << "Register builtin ui controls...";
        scene2d::ControlRegistry::get()->registerControl<scene2d::KmlControl>();
        scene2d::ControlRegistry::get()->registerControl<windows::control::LineEditControl>();
        scene2d::ControlRegistry::get()->registerControl<windows::control::ProgressBarControl>();
        scene2d::ControlRegistry::get()->registerControl<windows::control::ImageControl>();
        scene2d::ControlRegistry::get()->registerControl<windows::control::ButtonControl>();
        scene2d::ControlRegistry::get()->registerControl<windows::control::ImageButtonControl>();
    }

    Application* q;
};

Application::Application(int argc, char* argv[])
    : d(new Private(this))
{
    d->init();
}

Application::Application(int argc, wchar_t* argv[])
    : d(new Private(this))
{
    d->init();
}

Application::~Application()
{
    delete d;

    ScriptEngine::release();
    windows::graphics::GraphicDevice::releaseInstance();
    windows::ResourceManager::releaseInstance();

    CoUninitialize();
}

void Application::setLogCallback(LogCallback callback)
{
    g_log_callback = callback;
}

bool Application::scriptReloadEnabled()
{
    return g_script_reload_enabled;
}

void Application::enableScriptReload(bool enable)
{
    g_script_reload_enabled = enable;
}

bool Application::preloadResourceArchive(int id)
{
    return windows::ResourceManager::instance()->preloadResourceArchive(id);
}

void Application::setResourceRootDir(const char* dir)
{
    windows::ResourceManager::instance()->setResourceRootDir(dir);
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

void Application::quit()
{
    ::PostQuitMessage(0);
}

}
