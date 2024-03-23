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
#include "windows/control/SpinnerControl.h"
#include "windows/EncodingManager.h"
#include "windows/ResourceManager.h"
#include "windows/HiddenMsgWindow.h"
#include "resources/resources.h"
#include <ConsoleApi2.h>

namespace kwui {

static Application* g_app = nullptr;
static LogCallback g_log_callback = nullptr;
#ifdef NDEBUG
static bool g_script_reload_enabled = false;
#else
static bool g_script_reload_enabled = true;
#endif
static DWORD g_main_thread_id = 0;

class Application::Private : windows::WindowMsgListener {
public:
    Private(Application* q)
        : q_(q)
    {
        g_app = q;
        msg_window_.setListener(this);
        g_main_thread_id = ::GetCurrentThreadId();
    }
    ~Private()
    {
        msg_window_.setListener(nullptr);
        g_app = nullptr;
    }
    void onAppMessage(WPARAM wParam, LPARAM lParam) override
    {
        auto f = (std::function<void()>*)((void*)wParam);
        if (f) {
            (*f)();
            delete f;
        }
    }
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
    }

    Application* q_;
    windows::HiddenMsgWindow msg_window_;
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

bool Application::isMainThread()
{
    return (::GetCurrentThreadId() == g_main_thread_id);
}
void Application::runInMainThread(std::function<void()>&& func)
{
    if (!g_app)
        return;
    ::PostMessageW(
        g_app->d->msg_window_.getHwnd(),
        windows::HiddenMsgWindow::MESSAGE_TYPE,
        (WPARAM)new std::function<void()>(std::move(func)),
        NULL);
}
bool Application::preloadResourceArchive(int id)
{
    return windows::ResourceManager::instance()->preloadResourceArchive(id);
}

void Application::setResourceRootDir(const char* dir)
{
    windows::ResourceManager::instance()->setResourceRootDir(dir);
}
void Application::setResourceRootData(const uint8_t* data, size_t len)
{
    windows::ResourceManager::instance()->setResourceRootData(data, len);
}
void Application::addFont(const char* family_name, const char* font_path)
{
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
