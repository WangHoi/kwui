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
#include "src/gpu/ganesh/gl/GrGLUtil.h"

#include <Windows.h>
#include <GL/gl.h>

using sk_app::GLWindowContext;
using sk_app::DisplayParams;

#ifndef GL_TEXTURE_CUBE_MAP_SEAMLESS
#define GL_TEXTURE_CUBE_MAP_SEAMLESS 0x884F
#endif

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
                                kGLPreferCoreProfile_SkWGLContextRequest, fSharedHGLRC);
    if (nullptr == fHGLRC) {
        return nullptr;
    }

    SkWGLExtensions extensions;
    if (extensions.hasExtension(dc, "WGL_EXT_swap_control")) {
        extensions.swapInterval(fDisplayParams.fDisableVsync ? 0 : 1);
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

    auto gl_ifce = GrGLMakeNativeInterface();
    if (gl_ifce)
    {
        auto driver = GrGLGetDriverInfo(gl_ifce.get());
        bool srgb_framebuffer = driver.fVersion >= GR_GL_VER(3, 0) ||
                                extensions.hasExtension(dc, "GL_ARB_framebuffer_sRGB") ||
                                extensions.hasExtension(dc, "GL_EXT_framebuffer_sRGB");
        if (srgb_framebuffer)
            glEnable(GR_GL_FRAMEBUFFER_SRGB);

        bool seamless_cubemap = driver.fVersion >= GR_GL_VER(3, 2) ||
                                extensions.hasExtension(dc, "GL_ARB_seamless_cube_map");
        if (seamless_cubemap)
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    }
    return gl_ifce;
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
