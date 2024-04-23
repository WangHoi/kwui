#include "SurfaceAndroid.h"

#include <android/bitmap.h>
#include <android/log.h>

#include "tools/sk_app/Application.h"
#include "tools/sk_app/DisplayParams.h"
#include "tools/sk_app/android/WindowContextFactory_android.h"

namespace android {

WindowSurface::WindowSurface(ANativeWindow* win, std::unique_ptr<sk_app::WindowContext> wctx)
    : fWindow(win)
    , fWindowContext(std::move(wctx))
{
    SkASSERT(fWindow);
    SkASSERT(fWindowContext);

    fSurface = fWindowContext->getBackbufferSurface();
}

void WindowSurface::release(JNIEnv* env) {
    fWindowContext.reset();
    ANativeWindow_release(fWindow);
}

SkCanvas* WindowSurface::getCanvas() {
    if (fSurface) {
        return fSurface->getCanvas();
    }
    return nullptr;
}

void WindowSurface::flushAndSubmit() {
    fSurface->flushAndSubmit();
    fWindowContext->swapBuffers();
    fSurface = fWindowContext->getBackbufferSurface();
}

}  // namespace
