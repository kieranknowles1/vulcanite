#include "VkBootstrap.h"
#include "utility.hpp"
#include "vulkanhandle.hpp"
#include <SDL3/SDL_vulkan.h>
#include <fmt/base.h>
#include <vulkan/vulkan_core.h>

#define VMA_DEBUG_LOG_FORMAT(format, ...)                                      \
  do {                                                                         \
    printf((format), __VA_ARGS__);                                             \
    printf("\n");                                                              \
  } while (false)

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace selwonk::vulkan {
void VulkanHandle::init(Settings settings, glm::uvec2 windowSize,
                        SDL_Window *window) {
  fmt::println("Initialising Vulkan");

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
  VkSurfaceKHR surface;
  SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(mInstance),
                           /*allocator=*/nullptr, &surface);
  mSurface = surface;

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

  // Create a queue that can handle everything we need
  mGraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
  mGraphicsQueueFamily =
      vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

  VmaAllocatorCreateInfo allocInfo = {
      // Allow raw buffer access via pointers
      .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
      .physicalDevice = mPhysicalDevice,
      .device = mDevice,
      .instance = mInstance,
  };
  vmaCreateAllocator(&allocInfo, &mAllocator);
};

void VulkanHandle::initSwapchain(glm::uvec2 windowSize) {
  vkb::SwapchainBuilder builder(mPhysicalDevice, mDevice, mSurface);
  // 24-bit color depth + alpha channel
  mSwapchainFormat = ::vk::Format::eB8G8R8A8Unorm;

  vkb::Swapchain vkbSwapchain =
      builder
          .set_desired_format(
              {.format = static_cast<VkFormat>(mSwapchainFormat),
               .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
          // Hard v-sync, limiting FPS to display refresh rate
          .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
          .set_desired_extent(windowSize.x, windowSize.y)
          // We will transfer data directly to the swapchain image
          .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
          .build()
          .value();

  mSwapchainExtent =
      vk::Extent3D(vkbSwapchain.extent.width, vkbSwapchain.extent.height, 1);
  mSwapchain = vkbSwapchain.swapchain;

  auto images = vkbSwapchain.get_images().value();
  auto views = vkbSwapchain.get_image_views().value();
  assert(images.size() == views.size());

  mSwapchainEntries.reserve(images.size());
  for (int i = 0; i < images.size(); i++) {
    mSwapchainEntries.push_back({images[i], views[i], createSemaphore()});
  }
}

void VulkanHandle::shutdown() {
  vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);

  for (int i = 0; i < mSwapchainEntries.size(); i++) {
    vkDestroyImageView(mDevice, mSwapchainEntries[i].view, nullptr);
    destroySemaphore(mSwapchainEntries[i].semaphore);
  }

  vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
  vmaDestroyAllocator(mAllocator);
  vkDestroyDevice(mDevice, nullptr);
  // VkPhysicalDevice can't be destroyed, because it's really a handle. Ditto
  // for VkQueue
  vkb::destroy_debug_utils_messenger(mInstance, mDebugMessenger);
  vkDestroyInstance(mInstance, nullptr);
}

VkSemaphore VulkanHandle::createSemaphore(VkSemaphoreCreateFlags flags) {
  auto semInfo = VkSemaphoreCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = flags,
  };
  VkSemaphore result;
  check(vkCreateSemaphore(mDevice, &semInfo, nullptr, &result));
  return result;
}

void VulkanHandle::destroySemaphore(VkSemaphore sem) {
  vkDestroySemaphore(mDevice, sem, nullptr);
}
} // namespace selwonk::vulkan
