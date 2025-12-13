#pragma once

#include <vector>

#include <SDL3/SDL.h>
#include <glm/ext/vector_int2.hpp>
#include <glm/ext/vector_uint2.hpp>
#include <vulkan/vulkan.hpp>

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

  vk::Instance mInstance; // Main handle to the Vulkan library
  vk::DebugUtilsMessengerEXT mDebugMessenger; // Debug output handle
  vk::PhysicalDevice mPhysicalDevice;         // GPU for the device
  vk::Device mDevice;                         // Logical device for the GPU
  vk::SurfaceKHR mSurface;                    // Window that we render to

  vk::SwapchainKHR mSwapchain; // Double-buffered image queue
  vk::Format mSwapchainFormat; // Format of the swapchain images
  glm::uvec2 mSwapchainExtent; // Dimensions of the swapchain images. May differ
                               // from what was requested
  std::vector<vk::Image> mSwapchainImages; // Framebuffers for rendering targets
  std::vector<vk::ImageView> mSwapchainImageViews; // Magic views for magic shit

  vk::Queue mGraphicsQueue;
  uint32_t mGraphicsQueueFamily;

private:
  void initVulkan(Settings settings, SDL_Window *window);
  void initSwapchain(glm::ivec2 windowSize);
};
} // namespace selwonk::vul
