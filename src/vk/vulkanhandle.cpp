#include "VkBootstrap.h"
#include "vulkanhandle.hpp"
#include <SDL3/SDL_vulkan.h>

namespace selwonk::vk {
void VulkanHandle::init(Settings settings, SDL_Window *window) {
  // Vulkan instance
  vkb::InstanceBuilder builder;
  auto instResult =
      builder.set_app_name("Vulcanite")
          .request_validation_layers(settings.mRequestValidationLayers)
          .use_default_debug_messenger()
          .require_api_version(MinVulkanMajor, MinVulkanMinor, MinVulkanPatch)
          .build();

  vkb::Instance vkbInstance = instResult.value();
  mInstance = vkbInstance.instance;
  mDebugMessenger = vkbInstance.debug_messenger;

  // Vulkan surface
  // TODO: Maybe we want to use vma alloc for this
  SDL_Vulkan_CreateSurface(window, mInstance, /*allocator=*/nullptr, &mSurface);

  VkPhysicalDeviceVulkan13Features features13 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .synchronization2 = true,
      .dynamicRendering = true,
  };
  VkPhysicalDeviceVulkan12Features features12 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
      .descriptorIndexing = true,
      .bufferDeviceAddress = true,
  };

  // Select a GPU that supports our requirements
  vkb::PhysicalDeviceSelector selector(vkbInstance);
  vkb::PhysicalDevice vkbPhysicalDevice =
      selector.set_minimum_version(MinVulkanMajor, MinVulkanMinor)
          .set_required_features_13(features13)
          .set_required_features_12(features12)
          .set_surface(mSurface)
          .select()
          .value();

  vkb::DeviceBuilder deviceBuilder(vkbPhysicalDevice);
  vkb::Device vkbDevice = deviceBuilder.build().value();

  mDevice = vkbDevice.device;
  mPhysicalDevice = vkbPhysicalDevice.physical_device;
};

void VulkanHandle::shutdown() {
  // TODO: Implement
}
} // namespace selwonk::vk
