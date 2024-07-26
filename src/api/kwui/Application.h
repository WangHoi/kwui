#pragma once
#include "kwui_export.h"
#include "kwui_main.h"
#include <functional>

#ifdef __ANDROID__
#include <jni.h>
#endif

namespace kwui
{
typedef void (*LogCallback)(const char* msg);

enum NativeViewType
{
    NATIVE_VIEW_UNSUPPORTED,
    NATIVE_VIEW_D3D11,
};

class KWUI_EXPORT NativeViewHandler
{
public:
    virtual ~NativeViewHandler() {}
    virtual void onPaint(void* surface, float width, float height) {}
    virtual void onSetAttribute(const char* name, const std::string* value) {}
};

class KWUI_EXPORT Application
{
public:
    Application(int argc, char* argv[]);
    Application(int argc, wchar_t* argv[]);
    ~Application();
    static void setLogCallback(LogCallback callback);
    static bool scriptReloadEnabled();
    static void enableScriptReload(bool enable);
    static bool isMainThread();
    static void runInMainThread(std::function<void()>&& func);
    bool preloadResourceArchive(int id);
    void setResourceRootDir(const char* dir);
    void setResourceRootData(const uint8_t* data, size_t len);
    void addFont(const char* family_name, const char* font_path);
    int exec();
    static void quit();

    void* getNativeViewData(NativeViewType& out_type);
    void setNativeViewHandler(NativeViewHandler* handler);

private:
    class Private;
    Private* d;
};
}
