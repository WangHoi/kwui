#include "Application.h"
#include "ScriptEngine.h"
#include "scene2d/KmlControl.h"
#include "scene2d/ButtonControl.h"
#include "scene2d/ImageControl.h"
#include "resources/resources.h"
#ifdef _WIN32
#include "windows/graphics/GraphicDevice.h"
#include "windows/Dialog.h"
#include "windows/control/Control.h"
#include "windows/control/LineEditControl.h"
#include "windows/control/ProgressBarControl.h"
#include "windows/control/SpinnerControl.h"
#include "windows/HiddenMsgWindow.h"
#include <ConsoleApi2.h>
#endif
#include "base/EncodingManager.h"
#include "base/ResourceManager.h"
#include "graph2d/graph2d.h"
#if WITH_SKIA
#include "xskia/GraphicDeviceX.h"
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

#ifdef __ANDROID__
    void init(JNIEnv* env, jobject asset_manager)
#else
    void init()
#endif
    {
#ifdef _WIN32
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        ::SetConsoleOutputCP(65001);

        MSG msg;
        PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE);
        //::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif
        base::initialize_log(g_log_callback);

        LOG(INFO) << "Init ResourceManager...";
#ifdef __ANDROID__
        base::ResourceManager::createInstance(env, asset_manager);
#else
        base::ResourceManager::createInstance();
#endif

#if WITH_SKIA
        LOG(INFO) << "Init GraphicDevice...";
        xskia::GraphicDeviceX::createInstance();
#else
#ifdef _WIN32
        LOG(INFO) << "Init GraphicDevice...";
        windows::graphics::GraphicDevice::createInstance()->Init();
#else
#pragma message("TODO: Application::Private::init().")
#endif
#endif

        LOG(INFO) << "Register builtin ui controls...";
        scene2d::ControlRegistry::get()->registerControl<scene2d::KmlControl>();
        scene2d::ControlRegistry::get()->registerControl<scene2d::ImageControl>();
        scene2d::ControlRegistry::get()->registerControl<scene2d::ButtonControl>();
#if !WITH_SKIA && defined(_WIN32)
        scene2d::ControlRegistry::get()->registerControl<windows::control::LineEditControl>();
        scene2d::ControlRegistry::get()->registerControl<windows::control::ProgressBarControl>();
        scene2d::ControlRegistry::get()->registerControl<windows::control::SpinnerControl>();
#endif

        LOG(INFO) << "Register builtin icon font...";
        auto icon_font = resources::get_icon_data();
        graph2d::addFont("kwui", icon_font.data(), icon_font.size());
    }

    Application* q_;
#ifdef _WIN32
    windows::HiddenMsgWindow msg_window_;
#else
#pragma message("TODO: implement platform specific HiddenMsgWindow.")
#endif
};

#ifdef __ANDROID__
Application::Application(JNIEnv* env, jobject asset_manager)
    : d(new Private(this))
{
    d->init(env, asset_manager);
}
#else
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
#endif

Application::~Application()
{
    delete d;

    ScriptEngine::release();

#if WITH_SKIA
    xskia::GraphicDeviceX::releaseInstance();
#else
#ifdef _WIN32
    windows::graphics::GraphicDevice::releaseInstance();
#else
#pragma message("TODO: GraphicDevice::releaseInstance().")
#endif
#endif

    base::ResourceManager::releaseInstance();

#ifdef _WIN32
    CoUninitialize();
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
    return base::ResourceManager::instance()->preloadResourceArchive(id);
#else
#pragma message("TODO: Application::preloadResourceArchive().")
    return false;
#endif
}

void Application::setResourceRootDir(const char* dir)
{
#ifdef _WIN32
    base::ResourceManager::instance()->setResourceRootDir(dir);
#else
#pragma message("TODO: Application::setResourceRootDir().")
#endif
}
void Application::setResourceRootData(const uint8_t* data, size_t len)
{
#ifdef _WIN32
    base::ResourceManager::instance()->setResourceRootData(data, len);
#else
#pragma message("TODO: Application::setResourceRootData().")
#endif
}
void Application::addFont(const char* family_name, const char* font_path)
{
    std::wstring u16_font_path
        = base::EncodingManager::UTF8ToWide(font_path);
    auto res = base::ResourceManager::instance()
        ->loadResource(u16_font_path.c_str());
    if (res.has_value()) {
        graph2d::addFont(family_name, res.value().data, res.value().size);
    } else {
        LOG(ERROR) << "addFont: failed to load resource [" << font_path << "]";
    }
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
