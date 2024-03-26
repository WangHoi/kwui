
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "html/html.h"

#include "tools/sk_app/VulkanSharedWindowContext.h"

#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkAutoMalloc.h"

#include "include/gpu/vk/GrVkExtensions.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/ganesh/vk/GrVkImage.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR
// windows wants to define this as CreateSemaphoreA or CreateSemaphoreW
#undef CreateSemaphore
#endif
#define GET_DEV_PROC(F) f##F = (PFN_vk##F)backendContext.fGetProc("vk" #F, VK_NULL_HANDLE, fDevice)

namespace sk_app {

VulkanSharedWindowContext::VulkanSharedWindowContext(const DisplayParams& displayParams,
                                                     const GrVkBackendContext& backendCtx,
                                                     SciterWindowVulkanBridge* pvkb,
                                                     const int imageWidth,
                                                     const int imageHeight)
        : WindowContext(displayParams)
        , fImageCount(pvkb->vkImageCount)
        , fCurrentBackbufferIndex(pvkb->vkImageCount)
        , fInstance(backendCtx.fInstance)
        , fDevice(backendCtx.fDevice)
        , fImages(pvkb->vkImages)
        , fSurfaceFormat(pvkb->vkImageFormat)
        , fImageUsageFlags(pvkb->vkImageUsageFlags)
        , fImageSharingMode(pvkb->vkImageSharingMode)
        , fGraphicsQueueIndex(backendCtx.fGraphicsQueueIndex)
        , fTransitionReturnLayout(pvkb->vkTransitionReturnLayout)
        , fReturnImageLayout(pvkb->vkReturnImageLayout)
        , pvkb(pvkb) {
    fWidth = imageWidth;
    fHeight = imageHeight;
    fSampleCount = std::max(1, fDisplayParams.fMSAASampleCount);
    fStencilBits = 8;
    this->initializeContext(backendCtx);
}

VulkanSharedWindowContext::~VulkanSharedWindowContext() { this->destroyContext(); }

void VulkanSharedWindowContext::setImageIndex(const uint32_t imageIndex) {
    fCurrentImageIndex = imageIndex;
}

void VulkanSharedWindowContext::resize(int w, int h) {
    pvkb->on_vk_size_change(w, h);
    pvkb->on_vk_image_change();
}

void VulkanSharedWindowContext::updateImages(const int imageWidth, const int imageHeight) {
    fImageCount = pvkb->vkImageCount;
    fImages = pvkb->vkImages;
    fSurfaceFormat = pvkb->vkImageFormat;
    fImageUsageFlags = pvkb->vkImageUsageFlags;
    fImageSharingMode = pvkb->vkImageSharingMode;

    fWidth = imageWidth;
    fHeight = imageHeight;

    fDeviceWaitIdle(fDevice);
    this->destroyBuffers();

    fDeviceWaitIdle(fDevice);
    this->createBuffers();
}

bool VulkanSharedWindowContext::isValid() { return fDevice != VK_NULL_HANDLE; }

void VulkanSharedWindowContext::setDisplayParams(const DisplayParams& params) {
    fDisplayParams = params;
}

void VulkanSharedWindowContext::initializeContext(const GrVkBackendContext& backendContext) {
    SkASSERT(!fContext);
    const GrVkExtensions extensions = *backendContext.fVkExtensions;

    if (!extensions.hasExtension(VK_KHR_SURFACE_EXTENSION_NAME, 25) ||
        !extensions.hasExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME, 68)) {
        return;
    }

    const auto localGetPhysicalDeviceProperties =
            reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(backendContext.fGetProc(
                    "vkGetPhysicalDeviceProperties", backendContext.fInstance, VK_NULL_HANDLE));
    if (!localGetPhysicalDeviceProperties) {
        return;
    }
    VkPhysicalDeviceProperties physDeviceProperties;
    localGetPhysicalDeviceProperties(backendContext.fPhysicalDevice, &physDeviceProperties);
    const uint32_t physDevVersion = physDeviceProperties.apiVersion;

    fInterface.reset(new GrVkInterface(backendContext.fGetProc,
                                       fInstance,
                                       fDevice,
                                       backendContext.fInstanceVersion,
                                       physDevVersion,
                                       &extensions));
    GET_DEV_PROC(DeviceWaitIdle);

    fContext = GrDirectContext::MakeVulkan(backendContext, fDisplayParams.fGrContextOptions);
    switch (fSurfaceFormat) {           // NOLINT(clang-diagnostic-switch-enum)
        case VK_FORMAT_R8G8B8A8_UNORM:  // fall through
        case VK_FORMAT_R8G8B8A8_SRGB:
            fSkSurfaceColorType = kRGBA_8888_SkColorType;
            break;
        case VK_FORMAT_B8G8R8A8_UNORM:
            fSkSurfaceColorType = kBGRA_8888_SkColorType;
            break;
        default:
            return;
    }

    if (!this->createBuffers()) {
        fDeviceWaitIdle(fDevice);
        destroyBuffers();
    }
}

bool VulkanSharedWindowContext::createBuffers() {
    // set up initial image layouts and create surfaces
    fImageLayouts.clear();
    fImageLayouts.resize(fImageCount);

    fSurfaces.clear();
    fSurfaces.resize(fImageCount);

    for (uint32_t i = 0; i < fImageCount; ++i) {
        fImageLayouts[i] = VK_IMAGE_LAYOUT_UNDEFINED;

        GrVkImageInfo info;
        info.fImage = fImages[i];
        info.fAlloc = GrVkAlloc();
        info.fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
        info.fFormat = fSurfaceFormat;
        info.fImageUsageFlags = fImageUsageFlags;
        info.fLevelCount = 1;
        info.fCurrentQueueFamily = fGraphicsQueueIndex;
        info.fSharingMode = fImageSharingMode;

        if (fImageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) {
            GrBackendTexture backendTexture(fWidth, fHeight, info);
            fSurfaces[i] = SkSurface::MakeFromBackendTexture(fContext.get(),
                                                             backendTexture,
                                                             kTopLeft_GrSurfaceOrigin,
                                                             fDisplayParams.fMSAASampleCount,
                                                             fSkSurfaceColorType,
                                                             fDisplayParams.fColorSpace,
                                                             &fDisplayParams.fSurfaceProps);
            fUsingBackendTexture = true;
        } else {
            if (fDisplayParams.fMSAASampleCount > 1) {
                return false;
            }
            GrBackendRenderTarget backendRT(fWidth, fHeight, fSampleCount, info);
            fSurfaces[i] = SkSurface::MakeFromBackendRenderTarget(fContext.get(),
                                                                  backendRT,
                                                                  kTopLeft_GrSurfaceOrigin,
                                                                  fSkSurfaceColorType,
                                                                  fDisplayParams.fColorSpace,
                                                                  &fDisplayParams.fSurfaceProps);
        }

        if (!fSurfaces[i]) {
            return false;
        }
    }

    // Set up the backbuffers
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = 0;

    // We create one additional backbuffer structure here, because we want to
    // give the command buffers they contain a chance to finish before we cycle back
    fBackbuffers.clear();
    fBackbuffers.resize(static_cast<uint64_t>(fImageCount) + 1);
    for (auto& backbuffer : fBackbuffers) {
        backbuffer.fImageIndex = -1;
        SkDEBUGCODE(VkResult result =) GR_VK_CALL(
                fInterface,
                CreateSemaphore(fDevice, &semaphoreInfo, nullptr, &backbuffer.fRenderSemaphore));
        SkASSERT(result == VK_SUCCESS);
    }
    fCurrentBackbufferIndex = fImageCount;
    return true;
}

void VulkanSharedWindowContext::destroyBuffers() {
    for (auto& backbuffer : fBackbuffers) {
        backbuffer.fImageIndex = -1;
        GR_VK_CALL(fInterface, DestroySemaphore(fDevice, backbuffer.fRenderSemaphore, nullptr));
    }

    fBackbuffers.clear();
    fSurfaces.clear();
    fImageLayouts.clear();
}

void VulkanSharedWindowContext::destroyContext() {
    if (this->isValid()) {
        fDeviceWaitIdle(fDevice);
        this->destroyBuffers();
    }

    if (fContext) {
        SkASSERT(fContext->unique());
        fContext.reset();
        fInterface.reset();
    }
}

VulkanSharedWindowContext::BackbufferInfo& VulkanSharedWindowContext::getAvailableBackbuffer() {
    SkASSERT(fBackbuffers.size() == static_cast<uint64_t>(fImageCount) + 1);

    ++fCurrentBackbufferIndex;
    if (fCurrentBackbufferIndex > fImageCount) {
        fCurrentBackbufferIndex = 0;
    }

    return fBackbuffers.at(fCurrentBackbufferIndex);
}

sk_sp<SkSurface> VulkanSharedWindowContext::getBackbufferSurface() {
    BackbufferInfo& backbuffer = getAvailableBackbuffer();
    backbuffer.fImageIndex = fCurrentImageIndex;
    SkSurface* surface = fSurfaces[backbuffer.fImageIndex].get();
    return sk_ref_sp(surface);
}

void VulkanSharedWindowContext::swapBuffers() {
    const BackbufferInfo& backbuffer = fBackbuffers.at(fCurrentBackbufferIndex);
    SkSurface* surface = fSurfaces[backbuffer.fImageIndex].get();
    if (fTransitionReturnLayout) {
        const GrBackendSurfaceMutableState returnState(fReturnImageLayout, fGraphicsQueueIndex);
        // GrFlushInfo{} can also be submitted with a semaphore if the client
        // requires it for waiting.
        surface->flush(GrFlushInfo{}, &returnState);
    } else {
        surface->flush(GrFlushInfo{});
    }
    surface->recordingContext()->asDirectContext()->submit();
}

void VulkanSharedWindowContext::setImageLayout(const uint32_t vkImageIndex,
                                               const VkImageLayout vkImageLayout) {
    // Skia needs to know the image layout before it can draw to the surface
    // Note that this will not execute until flushed
    if (fUsingBackendTexture) {
        fSurfaces[vkImageIndex]
                ->getBackendTexture(SkSurface::BackendHandleAccess::kFlushWrite_BackendHandleAccess)
                .setVkImageLayout(vkImageLayout);
    } else {
        fSurfaces[vkImageIndex]
                ->getBackendRenderTarget(
                        SkSurface::BackendHandleAccess::kFlushWrite_BackendHandleAccess)
                .setVkImageLayout(vkImageLayout);
    }
}

bool VulkanSharedWindowContext::isShared() const { return true; }

  namespace window_context_factory {

    std::unique_ptr<WindowContext> MakeVulkanShared(html::view* pv, const DisplayParams& params, SciterWindowVulkanBridge* pvkb) {
      // Initialize client vulkan context
      const auto window_dim = pv->client_dim();
      pvkb->on_vk_initialize(window_dim.x, window_dim.y);

      // Initialize backend context with existing vulkan handles
      GrVkBackendContext vkBackendCtx;
      vkBackendCtx.fInstance = pvkb->vkInstance;
      vkBackendCtx.fPhysicalDevice = pvkb->vkPhysicalDevice;
      vkBackendCtx.fDevice = pvkb->vkDevice;
      vkBackendCtx.fQueue = pvkb->vkGraphicsQueue;
      vkBackendCtx.fGraphicsQueueIndex = pvkb->vkGraphicsQueueIndex;
      vkBackendCtx.fMaxAPIVersion = pvkb->vkInstanceAPI;
      vkBackendCtx.fGetProc = pvkb->vkGetProc;

      // Initialize Skia Extensions
      if (pvkb->vkUseDeviceFeatures2) {
        vkBackendCtx.fDeviceFeatures2 = &pvkb->vkDeviceFeatures2;
      }
      else {
        vkBackendCtx.fDeviceFeatures = &pvkb->vkDeviceFeatures;
      }

      const auto skiaExtensions = std::make_unique<GrVkExtensions>();
      skiaExtensions->init(vkBackendCtx.fGetProc,
        vkBackendCtx.fInstance,
        vkBackendCtx.fPhysicalDevice,
        static_cast<uint32_t>(pvkb->vkInstanceExtensions.size()),
        pvkb->vkInstanceExtensions.data(),
        static_cast<uint32_t>(pvkb->vkDeviceExtensions.size()),
        pvkb->vkDeviceExtensions.data());
      vkBackendCtx.fVkExtensions = skiaExtensions.get();

      // Create Context
      auto ctx = std::make_unique<VulkanSharedWindowContext>(params,
        vkBackendCtx,
        pvkb,
        window_dim.x,
        window_dim.y);
      if (!ctx->isValid()) {
        return nullptr;
      }
      pvkb->set_custom_vulkan_context(ctx.get());
      return ctx;
    }
  }

}  // namespace sk_app
