/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSurface.h"
#include "src/core/SkAutoMalloc.h"
#include "tools/sk_app/RasterWindowContext.h"
#include "tools/sk_app/win/WindowContextFactory_win.h"

#include <Windows.h>

using sk_app::RasterWindowContext;
using sk_app::DisplayParams;

namespace {

class RasterWindowContext_win : public RasterWindowContext {
public:
    RasterWindowContext_win(HWND, const DisplayParams&);

    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;
    bool isValid() override { return SkToBool(fWnd); }
    void resize(int w, int h) override;
    void setDisplayParams(const DisplayParams& params) override;

protected:
    SkAutoMalloc fSurfaceMemory;
    sk_sp<SkSurface> fBackbufferSurface;
    HWND fWnd;

private:
    using INHERITED = RasterWindowContext;
};

RasterWindowContext_win::RasterWindowContext_win(HWND wnd, const DisplayParams& params)
    : INHERITED(params)
    , fWnd(wnd) {
    RECT rect;
    GetClientRect(wnd, &rect);
    this->resize(rect.right - rect.left, rect.bottom - rect.top);
}

void RasterWindowContext_win::setDisplayParams(const DisplayParams& params) {
    fDisplayParams = params;
    RECT rect;
    GetClientRect(fWnd, &rect);
    this->resize(rect.right - rect.left, rect.bottom - rect.top);
}

void RasterWindowContext_win::resize(int w, int h) {
    fWidth = w;
    fHeight = h;
    fBackbufferSurface.reset();
    const size_t bmpSize = sizeof(BITMAPINFOHEADER) + w * h * sizeof(uint32_t);
    fSurfaceMemory.reset(bmpSize);
    BITMAPINFO* bmpInfo = reinterpret_cast<BITMAPINFO*>(fSurfaceMemory.get());
    ZeroMemory(bmpInfo, sizeof(BITMAPINFO));
    bmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo->bmiHeader.biWidth = w;
    bmpInfo->bmiHeader.biHeight = -h; // negative means top-down bitmap. Skia draws top-down.
    bmpInfo->bmiHeader.biPlanes = 1;
    bmpInfo->bmiHeader.biBitCount = 32;
    bmpInfo->bmiHeader.biCompression = BI_RGB;
    void* pixels = bmpInfo->bmiColors;

    SkImageInfo info = SkImageInfo::Make(w, h, fDisplayParams.fColorType, kPremul_SkAlphaType,
                                         fDisplayParams.fColorSpace);
    fBackbufferSurface = SkSurface::MakeRasterDirect(info, pixels, sizeof(uint32_t) * w);
}

sk_sp<SkSurface> RasterWindowContext_win::getBackbufferSurface() { return fBackbufferSurface; }

class layered_window_ctx {
  POINT                   _source_pos;
  POINT                   _window_pos;
  SIZE                    _size;
  RECT                    _dirty;
  BLENDFUNCTION           _blend;
  UPDATELAYEREDWINDOWINFO _info;

  layered_window_ctx() {}

public:
  layered_window_ctx(int x, int y)
    : _source_pos(), _window_pos(), _blend(), _info() {
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

  void update(HWND window, HDC source /*, rect* pr*/) {

    if (_size.cx <= 0 || _size.cy <= 0) return;

    _info.hdcSrc = source;
#pragma TODO("partial update of layered?")
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


void RasterWindowContext_win::swapBuffers() {
    BITMAPINFO* bmpInfo = reinterpret_cast<BITMAPINFO*>(fSurfaceMemory.get());

    if (GetWindowLong(fWnd, GWL_EXSTYLE) & WS_EX_LAYERED) 
    {
      HDC dc = CreateCompatibleDC(NULL);
      LPVOID bits = NULL;
      HBITMAP hdib = CreateDIBSection(dc, bmpInfo, DIB_RGB_COLORS, &bits, NULL, 0);
      if (hdib) {
        memcpy(bits, bmpInfo->bmiColors, fWidth * fHeight * sizeof(uint32_t));
        HGDIOBJ hprev = SelectObject(dc, hdib);
        layered_window_ctx(fWidth, fHeight).update(fWnd, dc);
        SelectObject(dc, hprev);
        DeleteObject(hdib);
      }
      ReleaseDC(NULL, dc);
    }
    else {
      HDC dc = GetDC(fWnd);
      StretchDIBits(dc, 0, 0, fWidth, fHeight, 0, 0, fWidth, fHeight, bmpInfo->bmiColors, bmpInfo,
        DIB_RGB_COLORS, SRCCOPY);
      ReleaseDC(fWnd, dc);
    }
}

}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

std::unique_ptr<WindowContext> MakeRasterForWin(HWND wnd, const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new RasterWindowContext_win(wnd, params));
    if (!ctx->isValid()) {
        ctx = nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app
