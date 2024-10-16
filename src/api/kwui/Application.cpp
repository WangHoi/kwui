#include "Application.h"
#include "Application_private.h"
#include "ScriptEngine.h"
#include "resources/resources.h"
#include "scene2d/KmlControl.h"
#include "scene2d/ButtonControl.h"
#include "scene2d/ImageControl.h"
#include "scene2d/ProgressBarControl.h"
#include "scene2d/SpinnerControl.h"
#include "scene2d/LineEditControl.h"
#include "scene2d/CustomElementControl.h"
#ifdef _WIN32
#include "windows/graphics/GraphicDeviceD2D.h"
#include "windows/DialogWin32.h"
#include "windows/HiddenMsgWindow.h"
#include <ConsoleApi2.h>
#endif
#ifdef __ANDROID__
#include <unistd.h>
#include "android/Application_jni.h"
#endif
#include "base/EncodingManager.h"
#include "base/ResourceManager.h"
#include "graph2d/graph2d.h"
#if WITH_SKIA
#include "xskia/GraphicDeviceX.h"
#endif

namespace kwui
{
PaintSurfaceType g_surface_type_hint = PAINT_SURFACE_DEFAULT;
RenderCallback g_render_callback = nullptr;

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
#elif defined(__ANDROID__)
        g_main_thread_id = gettid();
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
        base::ResourceManager::createInstance(android::get_jni_env(),
            android::get_asset_manager());
#else
        base::ResourceManager::createInstance();
#endif

#if WITH_SKIA
        LOG(INFO) << "Init GraphicDevice...";
        xskia::GraphicDeviceX::createInstance();
        internal_data_.renderer_type = INTERNAL_RENDERER_OPENGL;
        internal_data_.context = nullptr;
#else
#ifdef _WIN32
        LOG(INFO) << "Init GraphicDevice...";
        windows::graphics::GraphicDeviceD2D::createInstance()->init();
        auto dev = windows::graphics::GraphicDeviceD2D::instance()->getD3DDevice1();
        if (dev) {
            internal_data_.renderer_type = INTERNAL_RENDERER_D3D11_1;
            internal_data_.context = dev;
        }
#else
#pragma message("TODO: Application::Private::init().")
#endif
#endif

        LOG(INFO) << "Register builtin ui controls...";
        scene2d::ControlRegistry::get()->registerControl<scene2d::KmlControl>();
        scene2d::ControlRegistry::get()->registerControl<scene2d::ImageControl>();
        scene2d::ControlRegistry::get()->registerControl<scene2d::ButtonControl>();
        scene2d::ControlRegistry::get()->registerControl<scene2d::ProgressBarControl>();
        scene2d::ControlRegistry::get()->registerControl<scene2d::SpinnerControl>();
        scene2d::ControlRegistry::get()->registerControl<scene2d::control::LineEditControl>();

        LOG(INFO) << "Register builtin icon font...";
        auto icon_font = resources::get_icon_data();
        graph2d::addFont("kwui", icon_font.data(), icon_font.size());
    }

    Application* q_;
    InternalData internal_data_ = {INTERNAL_RENDERER_UNKNOWN, nullptr};
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

#if WITH_SKIA
    xskia::GraphicDeviceX::releaseInstance();
#else
#ifdef _WIN32
    windows::graphics::GraphicDeviceD2D::releaseInstance();
#else
#pragma message("TODO: GraphicDevice::releaseInstance().")
#endif
#endif

    base::ResourceManager::releaseInstance();

#ifdef _WIN32
    CoUninitialize();
#endif
}

Application* Application::instance()
{
    return g_app;
}

void Application::setPaintSurfaceTypeHint(PaintSurfaceType type)
{
    g_surface_type_hint = type;
}

void Application::setLogCallback(LogCallback callback)
{
    g_log_callback = callback;
}

void Application::setRenderCallback(RenderCallback callback)
{
    g_render_callback = callback;
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
#elif defined(__ANDROID__)
    return (gettid() == g_main_thread_id);
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
#elif defined(__ANDROID__)
    android::run_in_main_thread(std::move(func));
#else
#pragma message("TODO: implement platform specific HiddenMsgWindow.")
#endif
}

bool Application::preloadResourceArchive(int id)
{
#ifdef _WIN32
    return base::ResourceManager::instance()->preloadResourceArchive(id);
#else
    LOG(ERROR) << "Application::preloadResourceArchive() not supported.";
    return false;
#endif
}

void Application::setResourceRootDir(const char* dir)
{
    base::ResourceManager::instance()->setResourceRootDir(dir);
}

void Application::setResourceRootData(const uint8_t* data, size_t len)
{
    base::ResourceManager::instance()->setResourceRootData(data, len);
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
    return 0;
#elif defined(__ANDROID__)
    return android::application_exec();
#else
#pragma message("TODO: Application::exec().")
    return 0;
#endif
}

void Application::quit()
{
#ifdef _WIN32
    ::PostQuitMessage(0);
#elif defined(__ANDROID__)
    LOG(ERROR) << "TODO: Application::quit().";
#else
#pragma message("TODO: Application::quit().")
#endif
}

bool Application::loadResource(const char* path, ResourceItem* resource_item)
{
    auto RM = base::ResourceManager::instance();
    if (!RM)
        return false;
    auto u16_path = base::EncodingManager::UTF8ToWide(path);
    auto item = RM->loadResource(u16_path.c_str());
    if (item.has_value()) {
        resource_item->data = item->data;
        resource_item->size = item->size;
        return true;
    }
    return false;
}

const InternalData* Application::internalData()
{
    return &d->internal_data_;
}

void Application::registerCustomElement(const char* name, CustomElementFactoryFn factory_fn)
{
    auto atom = base::string_intern(name);
    scene2d::ControlRegistry::get()
        ->registerControl(atom, scene2d::control::CustomElementContrlFactory(atom, factory_fn));
}
}
