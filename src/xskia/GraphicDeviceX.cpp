#include "GraphicDeviceX.h"
#include "include/core/SkStream.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkData.h"
#ifdef _WIN32
#include "include/ports/SkImageGeneratorWIC.h"
#endif
#ifdef __ANDROID__
#include "include/ports/SkImageGeneratorNDK.h"
#endif
#include <include/gpu/gl/GrGLInterface.h>

#include "absl/strings/match.h"
#include "resources/resources.h"
#include "base/ResourceManager.h"
#include "base/EncodingManager.h"
#include "base/log.h"
#include "GLWindowContextXWin32.h"

namespace {
#if defined(UNICODE)
#define STR_LIT(X) L## #X
#else
#define STR_LIT(X) #X
#endif

#define TEMP_CLASS STR_LIT("kwui_TempClass")

HWND create_temp_window() {
    HMODULE module = GetModuleHandle(nullptr);
    HWND wnd;
    RECT windowRect;
    windowRect.left = 0;
    windowRect.right = 8;
    windowRect.top = 0;
    windowRect.bottom = 8;

    WNDCLASS wc;

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC) DefWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = module;
    wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = TEMP_CLASS;

    if(!RegisterClass(&wc)) {
        return 0;
    }

    DWORD style, exStyle;
    exStyle = WS_EX_CLIENTEDGE;
    style = WS_SYSMENU;

    AdjustWindowRectEx(&windowRect, style, false, exStyle);
    if(!(wnd = CreateWindowEx(exStyle,
                              TEMP_CLASS,
                              STR_LIT("PlaceholderWindow"),
                              WS_CLIPSIBLINGS | WS_CLIPCHILDREN | style,
                              0, 0,
                              windowRect.right-windowRect.left,
                              windowRect.bottom-windowRect.top,
                              nullptr, nullptr,
                              module,
                              nullptr))) {
        UnregisterClass(TEMP_CLASS, module);
        return nullptr;
                              }
    ShowWindow(wnd, SW_HIDE);

    return wnd;
}

void destroy_temp_window(HWND wnd) {
    DestroyWindow(wnd);
    HMODULE module = GetModuleHandle(nullptr);
    UnregisterClass(TEMP_CLASS, module);
}
}

namespace xskia
{
static GraphicDeviceX* s_instance = nullptr;

GraphicDeviceX::~GraphicDeviceX()
{
#ifdef _WIN32
    root_context_ = nullptr;
    destroy_temp_window(temp_hwnd_);
#endif
}

GraphicDeviceX* GraphicDeviceX::createInstance()
{
    if (!s_instance)
    {
        s_instance = new GraphicDeviceX();
        SkGraphics::Init();
#ifdef _WIN32
        SkGraphics::SetImageGeneratorFromEncodedDataFactory(&SkImageGeneratorWIC::MakeFromEncodedWIC);
#endif
#ifdef __ANDROID__
		SkGraphics::SetImageGeneratorFromEncodedDataFactory(&SkImageGeneratorNDK::MakeFromEncodedNDK);
#endif

#ifdef _WIN32
        // Create offscreen root GLContext
        s_instance->temp_hwnd_ = create_temp_window();
        s_instance->root_context_ = MakeGLForWin(s_instance->temp_hwnd_, sk_app::DisplayParams(), nullptr);
        if (!s_instance->root_context_)
        {
            LOG(ERROR) << "wglMakeCurrent root GLContext failed.";
        }
#endif
    }
    return s_instance;
}

void GraphicDeviceX::releaseInstance()
{
    if (s_instance)
    {
        delete s_instance;
        s_instance = nullptr;
    }
}

GraphicDeviceX* GraphicDeviceX::instance()
{
    return s_instance;
}

void GraphicDeviceX::addFont(const char* family_name, const void* data, size_t size)
{
    sk_sp<SkTypeface> font_face = SkFontMgr::RefDefault()
        ->makeFromStream(SkMemoryStream::MakeCopy(data, size));
    if (font_face)
        font_cache_[family_name] = font_face;
}

sk_sp<SkTypeface> GraphicDeviceX::getFirstMatchingFontFace(
    const char* family_name,
    SkFontStyle style)
{
    if (family_name)
    {
        auto it = font_cache_.find(family_name);
        if (it != font_cache_.end())
            return it->second;
    }
    return SkTypeface::MakeFromName(family_name, style);
}

BitmapSubItemX GraphicDeviceX::getBitmap(const std::string& name, float dpi_scale)
{
    auto it = bitmap_cache_.find(name);
    if (it == bitmap_cache_.end())
    {
        loadBitmapToCache(name);
        it = bitmap_cache_.find(name);
    }
    if (it == bitmap_cache_.end())
        return BitmapSubItemX();
    BitmapSubItemX res;
    res.frame = it->second;
    res.dpi_scale = dpi_scale;
    return res;
}

#ifdef _WIN32
HGLRC GraphicDeviceX::rootGLContext() const
{
    return root_context_ ? root_context_->glrc() : nullptr;
}
#endif

void GraphicDeviceX::loadBitmapToCache(const std::string& name)
{
    if (absl::StartsWith(name, "kwui::"))
    {
        std::string name_res = name.substr(6);
        absl::optional<absl::Span<const uint8_t>> x1;
        x1 = resources::get_image_data(name_res);
        if (!x1.has_value())
            return;
        loadBitmapFromResource(name, x1.value());
    }
    else if (absl::StartsWith(name, ":"))
    {
        auto RM = base::ResourceManager::instance();
        std::string name_res = name.substr(1);
        absl::optional<base::ResourceArchive::ResourceItem> x1, x1_5, x2;
        x1 = RM->loadResource(base::EncodingManager::UTF8ToWide(name_res).c_str());
        if (!x1.has_value())
            return;
        loadBitmapFromResource(name, absl::MakeSpan(x1->data, x1->size));
    }
    else
    {
        FILE* f = nullptr;
#ifdef _WIN32
        auto filename_x1 = base::EncodingManager::UTF8ToWide(name);
        f = _wfopen(filename_x1.c_str(), L"rb");
#else
		f = fopen(name.c_str(), "rb");
#endif
        if (f)
        {
            sk_sp<SkImage> image = SkImage::MakeFromEncoded(SkData::MakeFromFILE(f));
            if (image)
            {
                bitmap_cache_[name] = image;
            }
            fclose(f);
        }
    }
}

void GraphicDeviceX::loadBitmapFromResource(const std::string& name, absl::Span<const uint8_t> res_x1)
{
    sk_sp<SkImage> image = SkImage::MakeFromEncoded(SkData::MakeWithoutCopy(res_x1.data(), res_x1.size()));
    if (image)
    {
        bitmap_cache_[name] = image;
    }
}
}
