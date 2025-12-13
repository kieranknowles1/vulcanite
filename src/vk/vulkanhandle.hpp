#pragma once

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

namespace selwonk::vk {
class VulkanHandle {
public:
  const static constexpr uint32_t MinVulkanMajor = 1;
  const static constexpr uint32_t MinVulkanMinor = 3;
  const static constexpr uint32_t MinVulkanPatch = 0;

  struct Settings {
    bool mRequestValidationLayers = true;
  };

  void init(Settings settings, SDL_Window *window);
  void shutdown();

private:
  VkInstance mInstance;                     // Main handle to the Vulkan library
  VkDebugUtilsMessengerEXT mDebugMessenger; // Debug output handle
  VkPhysicalDevice mPhysicalDevice;         // GPU for the device
  VkDevice mDevice;                         // Logical device for the GPU
  VkSurfaceKHR mSurface;                    // Window that we render to
};
} // namespace selwonk::vk
