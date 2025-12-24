#pragma once

#include <functional>
#include <vector>

#include "image.hpp"
#include "vulkan/vulkan.hpp"
#include <SDL3/SDL.h>
#include <glm/ext/vector_int2.hpp>
#include <glm/ext/vector_uint2.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace selwonk::vulkan {
class VulkanHandle {
public:
  const static constexpr uint32_t MinVulkanMajor = 1;
  const static constexpr uint32_t MinVulkanMinor = 3;
  const static constexpr uint32_t MinVulkanPatch = 0;

  struct Settings {
    bool mRequestValidationLayers = true;
  };

  void init(Settings settings, glm::uvec2 windowSize, SDL_Window *window);
  void shutdown();

  vk::Semaphore createSemaphore();
  vk::Fence createFence(bool signalled = false);
  void destroySemaphore(vk::Semaphore sem);
  void destroyFence(vk::Fence fence);

  // Submit and execute a command buffer immediately, blocks until completion
  // Prefer async submission if at all possible
  void immediateSubmit(std::function<void(vk::CommandBuffer cmd)> func);

  // TODO: Make private
  vk::Instance mInstance; // Main handle to the Vulkan library
  vk::DebugUtilsMessengerEXT mDebugMessenger; // Debug output handle
  vk::PhysicalDevice mPhysicalDevice;         // GPU for the device
  vk::Device mDevice;                         // Logical device for the GPU
  vk::SurfaceKHR mSurface;                    // Window that we render to

  // TODO: Maybe this should be part of the renderer
  vk::SwapchainKHR mSwapchain;   // Double-buffered image queue
  vk::Format mSwapchainFormat;   // Format of the swapchain images
  vk::Extent3D mSwapchainExtent; // Dimensions of the swapchain images. May
                                 // differ from what was requested
  vk::Extent2D swapchainExtent2d() {
    return {mSwapchainExtent.width, mSwapchainExtent.height};
  }

  struct SwapchainEntry {
    vk::Image image;
    vk::ImageView view;
    vk::Semaphore semaphore;
  };

  std::vector<SwapchainEntry>
      mSwapchainEntries; // Framebuffers for rendering targets

  vk::Queue mGraphicsQueue;
  uint32_t mGraphicsQueueFamily;

  VmaAllocator mAllocator;

private:
  void initVulkan(Settings settings, SDL_Window *window);
  void initSwapchain(glm::uvec2 windowSize);

  vk::Fence mImmediateFence;
  vk::CommandBuffer mImmediateCommandBuffer;
  vk::CommandPool mImmediateCommandPool;
};
} // namespace selwonk::vulkan
