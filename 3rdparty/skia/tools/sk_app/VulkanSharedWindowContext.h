
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef VulkanWindowContextShared_DEFINED
#define VulkanWindowContextShared_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_VULKAN

#include "include/gpu/vk/GrVkVulkan.h"

#include "include/gpu/vk/GrVkBackendContext.h"
#include "src/gpu/ganesh/vk/GrVkInterface.h"
#include "tools/gpu/vk/VkTestUtils.h"
#include "tools/sk_app/WindowContext.h"
#include "sdk.js/include/sciter-x-vulkan.h"

namespace html {
  class view;
}

namespace sk_app {

  /**
   * @brief This class is used to share a Vulkan context between Skia and the client.  Vulkan
   * initialization is handled by the client (a.k.a. host application).  Vulkan handles are shared between the client
   * and Skia.  For example, VkInstance, VkPhysicalDevice, VkDevice, etc.  For a complete
   * list of shared handles, see: GrVkBackendContext.  In order to use this class, the client
   * must supply at least one VkImage for Skia to render to.  Multiple images are also
   * supported.  These images must be initialized by the client. These images can be swapchain
   * images or any image the client creates.  These is no requirement for them to explicitly
   * be from the swapchain. Backend textures will be created from the supplied images and placed
   * into SkSurfaces.  This class must be destroyed before the client destroys Vulkan handles.
   *
   *
   * Image Creation:
   * One of the following formats must be used for the shared image(s):
   *   - VK_FORMAT_R8G8B8A8_UNORM
   *   - VK_FORMAT_R8G8B8A8_SRGB
   *   - VK_FORMAT_B8G8R8A8_UNORM
   *
   * The following flags must be used when creating the images:
   *   - VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
   *   - VK_IMAGE_USAGE_TRANSFER_SRC_BIT
   *   - VK_IMAGE_USAGE_TRANSFER_DST_BIT
   *   - if supported: VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR else VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
   *
   * The following flags are optional during image creation:
   *   - VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
   *   - VK_IMAGE_USAGE_SAMPLED_BIT
   *
   * Rebuilding Images:
   * This class must remake its backend surfaces whenever the shared images are remade by the client.
   * If the client remade the backing VkImages for any reason, the Skia backend textures and surfaces
   * will become invalid.  These need to be remade via the call to: updateImages()
   *
   * Clearing Images:
   * This class assumes images are cleared by the client
   *
   * Image Index:
   * The client must set the index of the image to draw to each frame.  For example,
   * if the client was directly rendering to swapchain images, this would be the image index
   * of the acquired swapchain image.  If the client only had a single image they were
   * rendering to, this would be zero.  This must be set before Skia draws via the call to:
   * setImageIndex()
   *
   * Image Layout:
   * Images can be in a variety of layouts both when the client uses the image and when Skia
   * uses the image.  Both need to know the image layout in order to handle the image.
   * When the client is finished with an image, it can let Skia know the image layout via
   * the call to: setImageLayout().  This notifies Skia of the current image layout.  Skia will
   * transition the image appropriately.  When Skia is done rendering the image, the client may
   * still require the image to be in a specific format.  The client can tell Skia to transition
   * the image layout to a specific format by setting: transitionReturnLayout and returnImageLayout
   * during class construction.  When Skia is done using the image, it will transition the
   * image layout to the desired format prior to the client using the image.
   *
   */

  class VulkanSharedWindowContext final : public WindowContext, public CustomVulkanContext {
  public:
      /**
       * @brief Shared vulkan context constructor
       * @param displayParams Color type, space, etc.
       * @param backendCtx Shared Vulkan handles
       * @param pvkb Sciter vulkan bridge that contains the Vulkan handles as well as the
       * implementation of handling size changes.
       * @param imageWidth Image width
       * @param imageHeight Image height
       */
      VulkanSharedWindowContext(const DisplayParams& displayParams,
                                const GrVkBackendContext& backendCtx,
                                SciterWindowVulkanBridge* pvkb,
                                const int imageWidth,
                                const int imageHeight);

      ~VulkanSharedWindowContext() override;
      VulkanSharedWindowContext(const VulkanSharedWindowContext& source) = delete;
      VulkanSharedWindowContext& operator=(const VulkanSharedWindowContext& source) = delete;
      VulkanSharedWindowContext(VulkanSharedWindowContext&& rhs) = delete;
      VulkanSharedWindowContext& operator=(VulkanSharedWindowContext&& rhs) = delete;

      /**
       * @brief Executes the drawing command to the current SkSurface.  The final VkImage will be
       * transitioned to the client's desired format if fTransitionReturnLayout has been set to true.
       */
      void swapBuffers() override;

      /**
       * @brief Returns true if fDevice is a non-null Vulkan handle
       */
      bool isValid() override;

      /**
       * @brief Called whenever the window is resized.  It is up to the client to implement this
       * method within SciterWindowVulkanBridge.  The client should resize their image(s) and then
       * update Skia via: UpdateImages()
       * @param w Width
       * @param h Height
       */
      void resize(int w, int h) override;

      /**
       * @brief Returns a reference to the backbuffer surface based on fCurrentImageIndex.
       * setImageIndex() must be called prior to this call.
       */
      sk_sp<SkSurface> getBackbufferSurface() override;

      /**
       * @brief Sets the index of the current active image to render to
       * @param imageIndex Current active image index to render to.
       */
      void setImageIndex(const uint32_t imageIndex) override;

      /**
       * @brief Set the current display parameters.
       * @param params Display params
       */
      void setDisplayParams(const DisplayParams& params) override;

      /**
       * @brief Notify skia of the current image layout.  Skia will handle the layout appropriately.
       * @param vkImageIndex Index into the array of images that the image layout applies to
       * @param vkImageLayout Layout to notify skia about.
       */
      void setImageLayout(const uint32_t vkImageIndex, const VkImageLayout vkImageLayout) override;

      /**
       * @brief Update the existing vkImages and SkSurfaces.  This must be called whenever the client
       * remakes the image(s)
       * @param imageWidth Image width
       * @param imageHeight Image height
       */
      void updateImages(const int imageWidth, const int imageHeight) override;

      /**
       * @brief Set to return true.  This ensures Skia does not clear the image.
       */
      bool isShared() const override;

  private:
      /**
       * @brief Initialize the Skia context that uses the shared Vulkan handles
       * @param backendContext Vulkan handles to share with Skia
       */
      void initializeContext(const GrVkBackendContext& backendContext);

      /**
       * @brief Destroy the Skia context.  This does not destroy the Vulkan context.  The client
       * is responsible for destroying the Vulkan context.
       */
      void destroyContext();

      /**
       * @brief Create Skia backend textures and SkSurfaces using the client supplied VkImages
       */
      bool createBuffers();

      /**
       * @brief Destroy the Skia backend textures and SkSurfaces.
       */
      void destroyBuffers();

      /**
       * @brief Skia backbuffer used for managing Skia surfaces. fRenderSemaphore is to be used for
       * queue operations. This is currently not used but should remain as it can be used in the
       * future for additional synchronization options if necessary.
       */
      struct BackbufferInfo {
          uint32_t fImageIndex;
          VkSemaphore fRenderSemaphore;
      };

      BackbufferInfo& getAvailableBackbuffer();
      std::vector<BackbufferInfo> fBackbuffers{};
      uint32_t fImageCount;
      uint32_t fCurrentBackbufferIndex;

      sk_sp<const GrVkInterface> fInterface;
      std::vector<sk_sp<SkSurface>> fSurfaces{};

      VkInstance fInstance = VK_NULL_HANDLE;
      VkDevice fDevice = VK_NULL_HANDLE;
      PFN_vkDeviceWaitIdle fDeviceWaitIdle = nullptr;

      VkImage* fImages;
      uint32_t fCurrentImageIndex{};
      std::vector<VkImageLayout> fImageLayouts{};

      VkFormat fSurfaceFormat{};
      VkImageUsageFlags fImageUsageFlags{};
      SkColorType fSkSurfaceColorType{};
      VkSharingMode fImageSharingMode{};
      uint32_t fGraphicsQueueIndex;

      bool fUsingBackendTexture{false};
      bool fTransitionReturnLayout{false};
      VkImageLayout fReturnImageLayout{};

      SciterWindowVulkanBridge* pvkb{nullptr};
  };

  namespace window_context_factory {

  /**
   * @brief Create a Vulkan context that is shared between the client and Skia backend
   * @param pv Sciter html::view class
   * @param params Display parameters
   * @param pvkb Sciter vulkan bridge that contains the Vulkan handles as well as the
   * implementation of handling size changes.
   */
     std::unique_ptr<WindowContext> MakeVulkanShared(html::view* pv, const DisplayParams& params, SciterWindowVulkanBridge* pvkb);
  }

}  // namespace sk_app

#endif  // SK_VULKAN

#endif
