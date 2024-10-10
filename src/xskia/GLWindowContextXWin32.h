#pragma once
#include <memory>
#include <tools/sk_app/DisplayParams.h>
#include <tools/sk_app/GLWindowContext.h>
#ifdef _WIN32
#include "windows/windows_header.h"
#endif

namespace xskia
{

#ifdef _WIN32
class GLWindowContextXWin32 : public sk_app::GLWindowContext
{
public:
    GLWindowContextXWin32(HWND, const sk_app::DisplayParams&, HGLRC);
    ~GLWindowContextXWin32() override;

    HGLRC glrc() const
    {
        return fHGLRC;
    }

    HGLRC sharedGLRC() const
    {
        return fSharedHGLRC;
    }

protected:
    void onSwapBuffers() override;

    sk_sp<const GrGLInterface> onInitializeContext() override;
    void onDestroyContext() override;

private:
    HWND fHWND;
    HGLRC fHGLRC;
    HGLRC fSharedHGLRC;

    using INHERITED = sk_app::GLWindowContext;
};

std::unique_ptr<GLWindowContextXWin32> MakeGLForWin(HWND hwnd, const sk_app::DisplayParams& params, HGLRC shared);

#endif

}
