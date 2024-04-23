#pragma once

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"

#include <jni.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>

#include "tools/sk_app/WindowContext.h"

#include "include/core/SkImage.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"

namespace android {

class WindowSurface final {
public:
    WindowSurface(ANativeWindow* win, std::unique_ptr<sk_app::WindowContext> wctx);

    void release(JNIEnv*);
    void flushAndSubmit();
    SkCanvas* getCanvas();

    int width()  const { return fSurface ? fSurface->width() : 0; }
    int height() const { return fSurface ? fSurface->height() : 0; }

    sk_sp<SkImage> makeImageSnapshot() const {
        return fSurface ? fSurface->makeImageSnapshot() : nullptr;
    }

private:
    sk_sp<SkSurface> fSurface;
    ANativeWindow*                         fWindow;
    std::unique_ptr<sk_app::WindowContext> fWindowContext;
};

}
