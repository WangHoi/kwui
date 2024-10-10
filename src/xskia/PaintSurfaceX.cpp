#include "PaintSurfaceX.h"

#include <tools/sk_app/DisplayParams.h>
#include <tools/sk_app/WindowContext.h>

#include "GLWindowContextXWin32.h"
#include "GraphicDeviceX.h"
#include "PainterX.h"
#ifdef __ANDROID__
#include "tools/sk_app/android/WindowContextFactory_android.h"
#endif
#ifdef _WIN32
#include "tools/sk_app/win/WindowContextFactory_win.h"
#endif

namespace xskia
{
#ifdef _WIN32
class layered_window_ctx
{
    POINT _source_pos;
    POINT _window_pos;
    SIZE _size;
    RECT _dirty;
    BLENDFUNCTION _blend;
    UPDATELAYEREDWINDOWINFO _info;

    layered_window_ctx()
    {
    }

public:
    layered_window_ctx(int x, int y)
        : _source_pos(), _window_pos(), _blend(), _info()
    {
        _size.cx = x;
        _size.cy = y;
        _info.cbSize = sizeof(_info);
        _info.pptSrc = &_source_pos;
        _info.pptDst = &_window_pos;
        _info.psize = &_size;
        _info.pblend = &_blend;
        _info.dwFlags = ULW_ALPHA;

        _blend.SourceConstantAlpha = 255;
        _blend.AlphaFormat = AC_SRC_ALPHA;
    }

    void update(HWND window, HDC source /*, rect* pr*/)
    {
        if (_size.cx <= 0 || _size.cy <= 0) return;

        _info.hdcSrc = source;
#pragma message("TODO: partial update of layered?")
        RECT rect;
        GetWindowRect(window, &rect);
        _window_pos.x = rect.left;
        _window_pos.y = rect.top;
        BOOL r = UpdateLayeredWindow(
            window, _info.hdcDst, const_cast<POINT*>(_info.pptDst),
            const_cast<SIZE*>(_info.psize), _info.hdcSrc,
            const_cast<POINT*>(_info.pptSrc), _info.crKey,
            const_cast<BLENDFUNCTION*>(_info.pblend), _info.dwFlags);
        assert(r);
        r = r;
    }
};
#endif

std::unique_ptr<PaintSurfaceX> PaintSurfaceX::create(const CreateInfo& config)
{
    return std::unique_ptr<PaintSurfaceX>(new PaintSurfaceX(config));
}

void PaintSurfaceX::resize(int pixel_width, int pixel_height, float dpi_scale)
{
    config_.pixel_size.width = pixel_width;
    config_.pixel_size.height = pixel_height;
    config_.dpi_scale = dpi_scale;
    createSurface();
}

std::unique_ptr<graph2d::PaintContextInterface> PaintSurfaceX::beginPaint()
{
#ifdef __ANDROID__
	if (!wnd_surface_)
		return nullptr;
	auto canvas = wnd_surface_->getCanvas();
	if (canvas) {
		return std::make_unique<PainterX>(this, canvas, config_.dpi_scale);
	} else {
		return nullptr;
	}
#else
    if (!surface_)
        return nullptr;
    return std::make_unique<PainterX>(this, surface_->getCanvas(), config_.dpi_scale);
#endif
}

bool PaintSurfaceX::endPaint()
{
#ifdef __ANDROID__
	if (!wnd_surface_)
		return false;
	if (auto canvas = wnd_surface_->getCanvas()) {
		canvas->flush();
	} else {
		return false;
	}
#else
    if (!surface_)
        return false;
    surface_->flush();
#endif
    return true;
}

void PaintSurfaceX::swapBuffers()
{
#ifdef __ANDROID__
	if (wnd_surface_)
		wnd_surface_->flushAndSubmit();
#else
    if (config_.surface_type == kwui::PAINT_SURFACE_X_RASTER) {
#ifdef _WIN32
        BITMAPINFO* bmpInfo = reinterpret_cast<BITMAPINFO*>(raster_buffer_.get());
        if (::GetWindowLong(config_.hwnd, GWL_EXSTYLE) & WS_EX_LAYERED) {
            HDC dc = CreateCompatibleDC(NULL);
            LPVOID bits = NULL;
            HBITMAP hdib = CreateDIBSection(dc, bmpInfo, DIB_RGB_COLORS, &bits, NULL, 0);
            if (hdib) {
                memcpy(bits, bmpInfo->bmiColors, config_.pixel_size.width * config_.pixel_size.height * sizeof(uint32_t));
                HGDIOBJ hprev = SelectObject(dc, hdib);
                layered_window_ctx(config_.pixel_size.width, config_.pixel_size.height).update(config_.hwnd, dc);
                SelectObject(dc, hprev);
                DeleteObject(hdib);
            }
            ReleaseDC(NULL, dc);
        } else {
            HDC dc = GetDC(config_.hwnd);
            StretchDIBits(dc,
                          0, 0,
                          config_.pixel_size.width,
                          config_.pixel_size.height,
                          0, 0,
                          config_.pixel_size.width,
                          config_.pixel_size.height,
                          bmpInfo->bmiColors, bmpInfo,
                          DIB_RGB_COLORS, SRCCOPY);
            ReleaseDC(config_.hwnd, dc);
        }
#endif
    } else if (config_.surface_type == kwui::PAINT_SURFACE_X_OPENGL) {
        surface_->flushAndSubmit();
    }
    if (wnd_context_)
        wnd_context_->swapBuffers();
#endif
}

void PaintSurfaceX::discardDeviceResources()
{
}

PaintSurfaceX::PaintSurfaceX(const CreateInfo& config)
    : config_(config)
{
    realizeSurfaceType();
    createSurface();
}

void PaintSurfaceX::realizeSurfaceType()
{
    if (config_.surface_type != kwui::PAINT_SURFACE_DEFAULT)
        return;
#ifdef __ANDROID__
	config_.surface_type = kwui::PAINT_SURFACE_X_OPENGL;
#elif _WIN32
    config_.surface_type = kwui::PAINT_SURFACE_X_OPENGL;
#else
	config_.surface_type = kwui::PAINT_SURFACE_X_RASTER;
#endif
}

void PaintSurfaceX::createSurface()
{
#ifdef __ANDROID__
	sk_app::DisplayParams params;
    params.fMSAASampleCount = 4;
    // params.fSurfaceProps = SkSurfaceProps(SkSurfaceProps::kDynamicMSAA_Flag, SkPixelGeometry::kUnknown_SkPixelGeometry);
    auto wnd_ctx = sk_app::window_context_factory::MakeGLForAndroid(config_.hwnd, params);
	if (wnd_ctx) {
		wnd_surface_.reset(new android::WindowSurface(config_.hwnd, std::move(wnd_ctx)));
		if (wnd_surface_ && wnd_surface_->getCanvas()) {
			wnd_surface_->getCanvas()->scale(config_.dpi_scale, config_.dpi_scale);
		}
	} else {
		LOG(ERROR) << "MakeGLForAndroid failed";
	}
#else
    const size_t w = (size_t)config_.pixel_size.width;
    const size_t h = (size_t)config_.pixel_size.height;
    SkImageInfo image_info = SkImageInfo::MakeN32Premul((int)w, (int)h);

#ifdef _WIN32
    if (config_.surface_type == kwui::PAINT_SURFACE_X_RASTER) {
        const size_t bmp_size = sizeof(BITMAPINFOHEADER) + w * h * sizeof(uint32_t);
        raster_buffer_.reset(new uint8_t[bmp_size]);

        BITMAPINFO* bmpInfo = reinterpret_cast<BITMAPINFO*>(raster_buffer_.get());
        ZeroMemory(bmpInfo, sizeof(BITMAPINFO));
        bmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmpInfo->bmiHeader.biWidth = (LONG)w;
        bmpInfo->bmiHeader.biHeight = -(LONG)h; // negative means top-down bitmap. Skia draws top-down.
        bmpInfo->bmiHeader.biPlanes = 1;
        bmpInfo->bmiHeader.biBitCount = 32;
        bmpInfo->bmiHeader.biCompression = BI_RGB;
        void* pixels = bmpInfo->bmiColors;

        surface_ = SkSurface::MakeRasterDirect(image_info, pixels, w * 4);
    } else if (config_.surface_type == kwui::PAINT_SURFACE_X_OPENGL) {
        sk_app::DisplayParams params;
        params.fMSAASampleCount = 4;
        // params.fSurfaceProps = SkSurfaceProps(SkSurfaceProps::kDynamicMSAA_Flag, SkPixelGeometry::kUnknown_SkPixelGeometry);
        auto root_ctx = GraphicDeviceX::instance()->rootGLContext();
        wnd_context_ = xskia::MakeGLForWin(config_.hwnd, params, root_ctx);
        if (wnd_context_)
            surface_ = wnd_context_->getBackbufferSurface();
    }
    auto canvas = surface_ ? surface_->getCanvas() : nullptr;
    if (canvas)
        canvas->scale(config_.dpi_scale, config_.dpi_scale);
#else
#pragma warning("TODO: PaintSurfaceX::createSurface().")
#endif
#endif // __ANDROID__
}

sk_sp<SkImage> PaintSurfaceX::getCachedBitmap(const std::string& key) const
{
    const auto it = bitmap_cache_.find(key);
    return (it == bitmap_cache_.end()) ? nullptr : it->second;
}

void PaintSurfaceX::updateCachedBitmap(const std::string& key, sk_sp<SkImage> bitmap)
{
    bitmap_cache_[key] = bitmap;
}
}
