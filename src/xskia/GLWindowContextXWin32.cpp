#include "GLWindowContextXWin32.h"

/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/gl/GrGLInterface.h"
#include "src/utils/win/SkWGL.h"
#include "tools/sk_app/GLWindowContext.h"
#include "tools/sk_app/win/WindowContextFactory_win.h"

#include <Windows.h>
#include <GL/gl.h>

using sk_app::GLWindowContext;
using sk_app::DisplayParams;

#if defined(_M_ARM64)

namespace xskia {
std::unique_ptr<WindowContext> MakeGLForWin(HWND, const DisplayParams&, HGLRC) { return nullptr; }
}  // namespace xskia

#else

namespace xskia
{
GLWindowContextXWin32::GLWindowContextXWin32(HWND wnd, const DisplayParams& params, HGLRC shared)
    : INHERITED(params)
      , fHWND(wnd)
      , fHGLRC(nullptr)
      , fSharedHGLRC(shared)
{
    // any config code here (particularly for msaa)?

    this->initializeContext();
}

GLWindowContextXWin32::~GLWindowContextXWin32()
{
    this->destroyContext();
}

sk_sp<const GrGLInterface> GLWindowContextXWin32::onInitializeContext()
{
    HDC dc = GetDC(fHWND);

    fHGLRC = SkCreateWGLContext(dc, fDisplayParams.fMSAASampleCount, false /* deepColor */,
                                kGLPreferCompatibilityProfile_SkWGLContextRequest, fSharedHGLRC);
    if (nullptr == fHGLRC) {
        return nullptr;
    }

    SkWGLExtensions extensions;
    if (extensions.hasExtension(dc, "WGL_EXT_swap_control")) {
        extensions.swapInterval(fDisplayParams.fDisableVsync ? 0 : 1);
    }

    // Look to see if RenderDoc is attached. If so, re-create the context with a core profile
    if (wglMakeCurrent(dc, fHGLRC)) {
        auto iface = GrGLMakeNativeInterface();
        bool renderDocAttached = iface->hasExtension("GL_EXT_debug_tool");
        iface.reset(nullptr);
        if (renderDocAttached) {
            wglDeleteContext(fHGLRC);
            fHGLRC = SkCreateWGLContext(dc, fDisplayParams.fMSAASampleCount, false /* deepColor */,
                                        kGLPreferCoreProfile_SkWGLContextRequest, fSharedHGLRC);
            if (nullptr == fHGLRC) {
                return nullptr;
            }
        }
    }

    if (wglMakeCurrent(dc, fHGLRC)) {
        glClearStencil(0);
        glClearColor(0, 0, 0, 0);
        glStencilMask(0xffffffff);
        glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        // use DescribePixelFormat to get the stencil and color bit depth.
        int pixelFormat = GetPixelFormat(dc);
        PIXELFORMATDESCRIPTOR pfd;
        DescribePixelFormat(dc, pixelFormat, sizeof(pfd), &pfd);
        fStencilBits = pfd.cStencilBits;

        // Get sample count if the MSAA WGL extension is present
        if (extensions.hasExtension(dc, "WGL_ARB_multisample")) {
            static const int kSampleCountAttr = SK_WGL_SAMPLES;
            extensions.getPixelFormatAttribiv(dc,
                                              pixelFormat,
                                              0,
                                              1,
                                              &kSampleCountAttr,
                                              &fSampleCount);
            fSampleCount = std::max(fSampleCount, 1);
        } else {
            fSampleCount = 1;
        }

        RECT rect;
        GetClientRect(fHWND, &rect);
        fWidth = rect.right - rect.left;
        fHeight = rect.bottom - rect.top;
        glViewport(0, 0, fWidth, fHeight);
    }
    return GrGLMakeNativeInterface();
}


void GLWindowContextXWin32::onDestroyContext()
{
    wglDeleteContext(fHGLRC);
    fHGLRC = NULL;
}


void GLWindowContextXWin32::onSwapBuffers()
{
    HDC dc = GetDC((HWND)fHWND);
    SwapBuffers(dc);
    ReleaseDC((HWND)fHWND, dc);
}

std::unique_ptr<GLWindowContextXWin32> MakeGLForWin(HWND wnd, const DisplayParams& params, HGLRC shared)
{
    std::unique_ptr<GLWindowContextXWin32> ctx(new GLWindowContextXWin32(wnd, params, shared));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}
} // namespace xskia

#endif
