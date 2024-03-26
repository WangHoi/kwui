/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef WindowContext_DEFINED
#define WindowContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "include/gpu/GrTypes.h"
#include "tools/sk_app/DisplayParams.h"

class GrDirectContext;
class SkSurface;
#ifdef SK_GRAPHITE_ENABLED
namespace skgpu::graphite {
class Context;
class Recorder;
}
#endif

#ifdef USE_VULKAN
  #include "include/gpu/vk/GrVkVulkan.h"
  #include "include/gpu/vk/GrVkBackendContext.h"
  #include "sdk.js/include/sciter-x-vulkan.h"
#endif // USE_VULKAN


namespace sk_app {

class WindowContext {
public:
    WindowContext(const DisplayParams&);

    virtual ~WindowContext();

    virtual sk_sp<SkSurface> getBackbufferSurface() = 0;

    virtual void swapBuffers() = 0;

    virtual bool isValid() = 0;

    virtual void resize(int w, int h) = 0;

    virtual void activate(bool isActive) {}

    virtual bool isShared() const { return false; }

    const DisplayParams& getDisplayParams() { return fDisplayParams; }
    virtual void setDisplayParams(const DisplayParams& params) = 0;

    GrDirectContext* directContext() const { return fContext.get(); }
#ifdef SK_GRAPHITE_ENABLED
    skgpu::graphite::Context* graphiteContext() const { return fGraphiteContext.get(); }
    skgpu::graphite::Recorder* graphiteRecorder() const { return fGraphiteRecorder.get(); }
#endif

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    SkISize dimensions() const { return {fWidth, fHeight}; }
    int sampleCount() const { return fSampleCount; }
    int stencilBits() const { return fStencilBits; }

    virtual bool isGpuContext() { return true; }

#ifdef USE_VULKAN
public:
    virtual bool getVulkanEnvironment(SciterVulkanEnvironment& env) { return false; }
    virtual bool getVulkanContext(SciterVulkanContext& ctx) { return false; }
#endif

protected:

    sk_sp<GrDirectContext> fContext;

    int               fWidth;
    int               fHeight;
    DisplayParams     fDisplayParams;

    // parameters obtained from the native window
    // Note that the platform .cpp file is responsible for
    // initializing fSampleCount and fStencilBits!
    int               fSampleCount = 1;
    int               fStencilBits = 0;
};

}   // namespace sk_app

#endif
