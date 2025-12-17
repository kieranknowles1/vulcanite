#pragma once

#include <vector>

#include "image.hpp"
#include <SDL3/SDL.h>
#include <glm/ext/vector_int2.hpp>
#include <glm/ext/vector_uint2.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace selwonk::vk {
class VulkanHandle {
public:
  const static constexpr uint32_t MinVulkanMajor = 1;
  const static constexpr uint32_t MinVulkanMinor = 3;
  const static constexpr uint32_t MinVulkanPatch = 0;

  void init(glm::uvec2 windowSize, SDL_Window *window);
  void shutdown();

  VkSemaphore createSemaphore(VkSemaphoreCreateFlags flags = 0);
  void destroySemaphore(VkSemaphore sem);

  VkInstance mInstance;                     // Main handle to the Vulkan library
  VkDebugUtilsMessengerEXT mDebugMessenger; // Debug output handle
  VkPhysicalDevice mPhysicalDevice;         // GPU for the device
  VkDevice mDevice;                         // Logical device for the GPU
  VkSurfaceKHR mSurface;                    // Window that we render to

  // TODO: Maybe this should be part of the renderer
  VkSwapchainKHR mSwapchain;   // Double-buffered image queue
  VkFormat mSwapchainFormat;   // Format of the swapchain images
  VkExtent3D mSwapchainExtent; // Dimensions of the swapchain images. May differ
                               // from what was requested
  std::vector<VkImage> mSwapchainImages; // Framebuffers for rendering targets
  std::vector<VkImageView> mSwapchainImageViews; // Magic views for magic shit
  std::vector<VkSemaphore> mRenderSemaphores;    // Swapchain image sync

  VkQueue mGraphicsQueue;
  uint32_t mGraphicsQueueFamily;

  VmaAllocator mAllocator;

private:
  void initVulkan(SDL_Window *window);
  void initSwapchain(glm::uvec2 windowSize);
};
} // namespace selwonk::vk
