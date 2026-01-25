#pragma once

#include <functional>
#include <set>
#include <vector>

#include "../core/settings.hpp"
#include "../core/singleton.hpp"
#include "../core/window.hpp"
#include "image.hpp"
#include "vulkan/vulkan.hpp"
#include <glm/ext/vector_int2.hpp>
#include <glm/ext/vector_uint2.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace selwonk::vulkan {
class VulkanHandle : public core::Singleton<VulkanHandle> {
public:
  const static constexpr uint32_t MinVulkanMajor = 1;
  const static constexpr uint32_t MinVulkanMinor = 3;
  const static constexpr uint32_t MinVulkanPatch = 0;

  VulkanHandle(core::Settings& settings, core::Window& window);
  ~VulkanHandle();

  vk::Semaphore createSemaphore();
  vk::Fence createFence(bool signalled = false);
  void destroySemaphore(vk::Semaphore sem);
  void destroyFence(vk::Fence fence);

  // Submit and execute a command buffer immediately, blocks until completion
  // Prefer async submission if at all possible
  void immediateSubmit(std::function<void(vk::CommandBuffer cmd)> func);

  // TODO: Make private
  const core::Settings& mSettings;

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

  void resizeSwapchain(glm::uvec2 newSize);

private:
  void
  onDebugMessage(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                 vk::DebugUtilsMessageTypeFlagsEXT type,
                 const vk::DebugUtilsMessengerCallbackDataEXT* callbackData);

  static VkBool32
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                VkDebugUtilsMessageTypeFlagsEXT type,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* pUserData);

  void logLimits();

  void initVulkan(core::Window& window);
  void destroySwapchain();
  void initSwapchain(glm::uvec2 windowSize);

  std::set<std::string> mSuppressedMessages = {
      // Harmless message that the first attempted driver isn't suitable
      "Loader Message",
      // TODO: Why is this triggering
      "VUID-VkDeviceCreateInfo-pNext-02830",
  };

  vk::Fence mImmediateFence;
  vk::CommandBuffer mImmediateCommandBuffer;
  vk::CommandPool mImmediateCommandPool;
};
} // namespace selwonk::vulkan
