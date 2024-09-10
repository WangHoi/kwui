#pragma once
#include "kwui_export.h"
#include "CustomElement.h"
#include <functional>
#include <string>

#ifdef __ANDROID__
#include <jni.h>
#endif

namespace kwui
{
typedef void (*LogCallback)(const char* msg);

enum InternalRendererType
{
    INTERNAL_RENDERER_UNKNOWN,
    INTERNAL_RENDERER_D3D11_1,
};

struct KWUI_EXPORT InternalData {
    InternalRendererType renderer_type;

    /// GL context, or D3D device
    void* context;
};

struct KWUI_EXPORT ResourceItem {
    const uint8_t* data = nullptr;
    size_t size = 0;
};

enum KWUI_EXPORT PaintSurfaceType
{
    PAINT_SURFACE_DEFAULT,
    PAINT_SURFACE_X_RASTER,
    PAINT_SURFACE_X_OPENGL,
    PAINT_SURFACE_X_VULKAN,
    PAINT_SURFACE_X_D3D10_1,
    PAINT_SURFACE_X_D3D11_1,
    PAINT_SURFACE_X_D3D12,
};

class KWUI_EXPORT Application
{
public:
    Application(int argc, char* argv[]);
    Application(int argc, wchar_t* argv[]);
    ~Application();
    static void setPaintSurfaceTypeHint(PaintSurfaceType type);
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

    static bool loadResource(const char* path, ResourceItem* resource_item);

    const InternalData* internalData();
    void registerCustomElement(const char* name, CustomElementFactoryFn factory_fn);

private:
    class Private;
    Private* d;
};
}
