#pragma once

#include <vector>

#include <SDL3/SDL.h>
#include <glm/ext/vector_int2.hpp>
#include <glm/ext/vector_uint2.hpp>
#include <vulkan/vulkan.h>

namespace selwonk::vul {
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

  VkInstance mInstance;                     // Main handle to the Vulkan library
  VkDebugUtilsMessengerEXT mDebugMessenger; // Debug output handle
  VkPhysicalDevice mPhysicalDevice;         // GPU for the device
  VkDevice mDevice;                         // Logical device for the GPU
  VkSurfaceKHR mSurface;                    // Window that we render to

  VkSwapchainKHR mSwapchain;   // Double-buffered image queue
  VkFormat mSwapchainFormat;   // Format of the swapchain images
  glm::uvec2 mSwapchainExtent; // Dimensions of the swapchain images. May differ
                               // from what was requested
  std::vector<VkImage> mSwapchainImages; // Framebuffers for rendering targets
  std::vector<VkImageView> mSwapchainImageViews; // Magic views for magic shit

  VkQueue mGraphicsQueue;
  uint32_t mGraphicsQueueFamily;

private:
  void initVulkan(Settings settings, SDL_Window *window);
  void initSwapchain(glm::ivec2 windowSize);
};
} // namespace selwonk::vul
