#include "Application.h"
#include "ScriptEngine.h"
#include "scene2d/KmlControl.h"
#include "resources/resources.h"
#ifdef _WIN32
#include "windows/graphics/GraphicDevice.h"
#include "windows/Dialog.h"
#include "windows/control/Control.h"
#include "windows/control/ButtonControl.h"
#include "windows/control/ImageControl.h"
#include "windows/control/ImageButtonControl.h"
#include "windows/control/LineEditControl.h"
#include "windows/control/ProgressBarControl.h"
#include "windows/control/SpinnerControl.h"
#include "windows/EncodingManager.h"
#include "windows/ResourceManager.h"
#include "windows/HiddenMsgWindow.h"
#include <ConsoleApi2.h>
#endif

namespace kwui {

static Application* g_app = nullptr;
static LogCallback g_log_callback = nullptr;
#ifdef NDEBUG
static bool g_script_reload_enabled = false;
#else
static bool g_script_reload_enabled = true;
#endif
static uint64_t g_main_thread_id = 0;

class Application::Private
#ifdef _WIN32
 : windows::WindowMsgListener
#else
#pragma message("TODO: implement WindowMsgListener.")
#endif
{
public:
    Private(Application* q)
        : q_(q)
    {
        g_app = q;
#if _WIN32
        msg_window_.setListener(this);
        g_main_thread_id = ::GetCurrentThreadId();
#else
#pragma message("TODO: GetCurrentThreadID().")
#endif
    }
    ~Private()
    {
#ifdef _WIN32
        msg_window_.setListener(nullptr);
#else
#pragma message("TODO: Application::Private::~Private().")
#endif
        g_app = nullptr;
    }
#ifdef _WIN32
    void onAppMessage(WPARAM wParam, LPARAM lParam) override
    {
        auto f = (std::function<void()>*)((void*)wParam);
        if (f) {
            (*f)();
            delete f;
        }
    }
#else
#pragma message("TODO: Application::Private::onAppMessage().")
#endif
    void init()
    {
        //::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        base::initialize_log(g_log_callback);
#ifdef _WIN32
        CoInitializeEx(NULL, COINIT_MULTITHREADED);

        ::SetConsoleOutputCP(65001);

        MSG msg;
        PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE);

        LOG(INFO) << "Init ResourceManager...";
        windows::ResourceManager::createInstance(::GetModuleHandleW(NULL));
        LOG(INFO) << "Init GraphicDevice...";
        windows::graphics::GraphicDevice::createInstance()->Init();

        LOG(INFO) << "Register builtin ui controls...";
        scene2d::ControlRegistry::get()->registerControl<scene2d::KmlControl>();
        scene2d::ControlRegistry::get()->registerControl<windows::control::LineEditControl>();
        scene2d::ControlRegistry::get()->registerControl<windows::control::ProgressBarControl>();
        scene2d::ControlRegistry::get()->registerControl<windows::control::ImageControl>();
        scene2d::ControlRegistry::get()->registerControl<windows::control::ButtonControl>();
        scene2d::ControlRegistry::get()->registerControl<windows::control::ImageButtonControl>();
        scene2d::ControlRegistry::get()->registerControl<windows::control::SpinnerControl>();

        LOG(INFO) << "Register builtin icon font...";
        auto icon_font = resources::get_icon_data();
        windows::graphics::GraphicDevice::instance()
            ->addFont("kwui", icon_font.data(), icon_font.size());
#else
#pragma message("TODO: Application::Private::init().")
#endif
    }

    Application* q_;
#ifdef _WIN32
    windows::HiddenMsgWindow msg_window_;
#else
#pragma message("TODO: implement platform specific HiddenMsgWindow.")
#endif
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
#ifdef _WIN32
    windows::graphics::GraphicDevice::releaseInstance();
    windows::ResourceManager::releaseInstance();

    CoUninitialize();
#else
#pragma message("TODO: Application::~Application().")
#endif
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

bool Application::isMainThread()
{
#if _WIN32
    return (::GetCurrentThreadId() == g_main_thread_id);
#else
#pragma message("TODO: GetCurrentThreadID().")
    return true;
#endif
}
void Application::runInMainThread(std::function<void()>&& func)
{
    if (!g_app)
        return;
#ifdef _WIN32
    ::PostMessageW(
        g_app->d->msg_window_.getHwnd(),
        windows::HiddenMsgWindow::MESSAGE_TYPE,
        (WPARAM)new std::function<void()>(std::move(func)),
        NULL);
#else
#pragma message("TODO: implement platform specific HiddenMsgWindow.")
#endif
}
bool Application::preloadResourceArchive(int id)
{
#ifdef _WIN32
    return windows::ResourceManager::instance()->preloadResourceArchive(id);
#else
#pragma message("TODO: Application::preloadResourceArchive().")
    return false;
#endif
}

void Application::setResourceRootDir(const char* dir)
{
#ifdef _WIN32
    windows::ResourceManager::instance()->setResourceRootDir(dir);
#else
#pragma message("TODO: Application::setResourceRootDir().")
#endif
}
void Application::setResourceRootData(const uint8_t* data, size_t len)
{
#ifdef _WIN32
    windows::ResourceManager::instance()->setResourceRootData(data, len);
#else
#pragma message("TODO: Application::setResourceRootData().")
#endif
}
void Application::addFont(const char* family_name, const char* font_path)
{
#ifdef _WIN32
    std::wstring u16_font_path
        = windows::EncodingManager::UTF8ToWide(font_path);
    auto res = windows::ResourceManager::instance()
        ->loadResource(u16_font_path.c_str());
    if (res.has_value()) {
        windows::graphics::GraphicDevice::instance()
            ->addFont(family_name, res.value().data, res.value().size);
    } else {
        LOG(ERROR) << "addFont: failed to load resource [" << font_path << "]";
    }
#else
#pragma message("TODO: Application::addFont().")
#endif
}

int Application::exec()
{
#ifdef _WIN32    
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#else
#pragma message("TODO: Application::exec().")
    return false;
#endif

    return 0;
}

void Application::quit()
{
#ifdef _WIN32
    ::PostQuitMessage(0);
#else
#pragma message("TODO: Application::quit().")
#endif
}

}
