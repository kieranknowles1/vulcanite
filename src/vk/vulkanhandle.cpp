#include "VkBootstrap.h"
#include "vulkanhandle.hpp"
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

namespace selwonk::vk {
void VulkanHandle::init(Settings settings, glm::uvec2 windowSize,
                        SDL_Window *window) {
  mSwapchainExtent = windowSize;

  initVulkan(settings, window);
  initSwapchain(windowSize);
};

void VulkanHandle::initVulkan(Settings settings, SDL_Window *window) {
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

void VulkanHandle::initSwapchain(glm::ivec2 windowSize) {
  vkb::SwapchainBuilder builder(mPhysicalDevice, mDevice, mSurface);
  // 24-bit color depth + alpha channel
  mSwapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;

  vkb::Swapchain vkbSwapchain =
      builder
          .set_desired_format({.format = mSwapchainFormat,
                               .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
          // Hard v-sync, limiting FPS to display refresh rate
          .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
          .set_desired_extent(windowSize.x, windowSize.y)
          // We will transfer data directly to the swapchain image
          .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
          .build()
          .value();

  mSwapchainExtent = {vkbSwapchain.extent.width, vkbSwapchain.extent.height};
  mSwapchain = vkbSwapchain.swapchain;
  mSwapchainImages = vkbSwapchain.get_images().value();
  mSwapchainImageViews = vkbSwapchain.get_image_views().value();
};

void VulkanHandle::shutdown() {
  vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);

  for (int i = 0; i < mSwapchainImageViews.size(); i++) {
    vkDestroyImageView(mDevice, mSwapchainImageViews[i], nullptr);
  }

  vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
  vkDestroyDevice(mDevice, nullptr);
  vkb::destroy_debug_utils_messenger(mInstance, mDebugMessenger);
  vkDestroyInstance(mInstance, nullptr);
}
} // namespace selwonk::vk
