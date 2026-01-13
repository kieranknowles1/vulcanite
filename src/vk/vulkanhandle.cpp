#include "../times.hpp"
#include "VkBootstrap.h"
#include "utility.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"
#include "vulkaninit.hpp"
#include <SDL3/SDL_vulkan.h>
#include <fmt/base.h>
#include <vulkan/vulkan_core.h>

namespace selwonk::vulkan {
VkBool32 VulkanHandle::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

  auto* self = static_cast<VulkanHandle*>(pUserData);
  self->onDebugMessage(
      static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(severity),
      static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(type),
      reinterpret_cast<const vk::DebugUtilsMessengerCallbackDataEXT*>(
          pCallbackData));

  return VK_FALSE; // Spec reserves VK_TRUE for future use
}

std::string_view
severityStr(vk::DebugUtilsMessageSeverityFlagBitsEXT severity) {
  using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;
  // These are flags, not a plain enum so we can't switch on them
  // If multiple are set, return the highest severity
  if (severity & eError)
    return "error";
  if (severity & eWarning)
    return "warning";
  if (severity & eInfo)
    return "info";
  if (severity & eVerbose)
    return "verbose";
  return "unknown";
}

std::string_view typeStr(vk::DebugUtilsMessageTypeFlagsEXT type) {
  using enum vk::DebugUtilsMessageTypeFlagBitsEXT;
  if (type & eValidation)
    return "validation";
  if (type & ePerformance)
    return "performance";
  if (type & eGeneral)
    return "general";
  return "unknown";
}

void VulkanHandle::onDebugMessage(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData) {
  if (mSuppressedMessages.find(pCallbackData->pMessageIdName) !=
      mSuppressedMessages.end())
    return;

  fmt::println("[vk:{} ({})] {}: {}", severityStr(severity), typeStr(type),
               pCallbackData->pMessageIdName, pCallbackData->pMessage);
}

VulkanHandle::VulkanHandle(core::Settings& settings, core::Window& window)
    : mSettings(settings) {
  fmt::println("Initialising Vulkan");

  initVulkan(settings.requestValidationLayers, window);
  initSwapchain(window.getSize());

  auto poolInfo = VulkanInit::commandPoolCreateInfo(mGraphicsQueueFamily);
  check(mDevice.createCommandPool(&poolInfo, nullptr, &mImmediateCommandPool));
  auto allocInfo = VulkanInit::bufferAllocateInfo(mImmediateCommandPool);
  check(mDevice.allocateCommandBuffers(&allocInfo, &mImmediateCommandBuffer));
  mImmediateFence = createFence(/*signalled=*/false);

  logLimits();
};

void VulkanHandle::initVulkan(bool requestValidationLayers,
                              core::Window& window) {
  vkb::InstanceBuilder builder;
  auto instResult =
      builder.set_app_name("Vulcanite")
          .request_validation_layers(requestValidationLayers)
          .set_debug_callback(debugCallback)
          .set_debug_callback_user_data_pointer(this)
          .require_api_version(MinVulkanMajor, MinVulkanMinor, MinVulkanPatch)
          .build();

  vkb::Instance vkbInstance = instResult.value();
  mInstance = vkbInstance.instance;
  mDebugMessenger = vkbInstance.debug_messenger;

  // Vulkan surface
  // TODO: Maybe we want to use vma alloc for this
  VkSurfaceKHR surface;
  SDL_Vulkan_CreateSurface(window.getSdl(), static_cast<VkInstance>(mInstance),
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
      .runtimeDescriptorArray = true,
      .bufferDeviceAddress = true,
  };
  VkPhysicalDeviceFeatures features = {
      .shaderInt64 = true,
  };

  // Select a GPU that supports our requirements
  vkb::PhysicalDeviceSelector selector(vkbInstance);
  vkb::PhysicalDevice vkbPhysicalDevice =
      selector.set_minimum_version(MinVulkanMajor, MinVulkanMinor)
          .set_required_features_13(features13)
          .set_required_features_12(features12)
          .set_required_features(features)
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

void VulkanHandle::resizeSwapchain(glm::uvec2 newSize) {
  check(mDevice.waitIdle());
  destroySwapchain();
  initSwapchain(newSize);
}

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
          .set_desired_present_mode((VkPresentModeKHR)mSettings.vsync)
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

void VulkanHandle::destroySwapchain() {
  mDevice.destroySwapchainKHR(mSwapchain, nullptr);

  for (int i = 0; i < mSwapchainEntries.size(); i++) {
    mDevice.destroyImageView(mSwapchainEntries[i].view, nullptr);
    destroySemaphore(mSwapchainEntries[i].semaphore);
  }
  mSwapchainEntries.clear();
}

void VulkanHandle::logLimits() {
  vk::PhysicalDeviceProperties props;
  mPhysicalDevice.getProperties(&props);

#define LOGLIMIT(limit) fmt::println(#limit ": {}", props.limits.limit)

  LOGLIMIT(maxPushConstantsSize);
  LOGLIMIT(minUniformBufferOffsetAlignment);
  LOGLIMIT(maxPerStageDescriptorSamplers);
  LOGLIMIT(maxPerStageDescriptorSampledImages);
}

VulkanHandle::~VulkanHandle() {
  destroySwapchain();

  mDevice.destroyCommandPool(mImmediateCommandPool, nullptr);
  mDevice.destroyFence(mImmediateFence, nullptr);

  vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
  vmaDestroyAllocator(mAllocator);
  vkDestroyDevice(mDevice, nullptr);
  // VkPhysicalDevice can't be destroyed, because it's really a handle. Ditto
  // for VkQueue
  vkb::destroy_debug_utils_messenger(mInstance, mDebugMessenger);
  vkDestroyInstance(mInstance, nullptr);
}

vk::Semaphore VulkanHandle::createSemaphore() {
  vk::SemaphoreCreateInfo semInfo{};
  vk::Semaphore result;
  check(mDevice.createSemaphore(&semInfo, nullptr, &result));
  return result;
}

vk::Fence VulkanHandle::createFence(bool signalled) {
  vk::FenceCreateInfo fenceInfo{};
  if (signalled)
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
  vk::Fence result;
  check(mDevice.createFence(&fenceInfo, nullptr, &result));
  return result;
}

void VulkanHandle::destroySemaphore(vk::Semaphore sem) {
  mDevice.destroySemaphore(sem, nullptr);
}

void VulkanHandle::destroyFence(vk::Fence fence) {
  mDevice.destroyFence(fence, nullptr);
}

void VulkanHandle::immediateSubmit(
    std::function<void(vk::CommandBuffer cmd)> func) {
  check(mDevice.resetFences(1, &mImmediateFence));
  check(mImmediateCommandBuffer.reset({}));

  auto beginInfo = VulkanInit::commandBufferBeginInfo(
      vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  check(mImmediateCommandBuffer.begin(&beginInfo));
  func(mImmediateCommandBuffer);
  check(mImmediateCommandBuffer.end());

  auto cmdInfo = VulkanInit::commandBufferSubmitInfo(mImmediateCommandBuffer);
  auto submitInfo = VulkanInit::submitInfo(&cmdInfo, nullptr, nullptr);
  check(mGraphicsQueue.submit2(1, &submitInfo, mImmediateFence));

  auto timeout = chronoToVulkan(std::chrono::seconds(1));
  check(mDevice.waitForFences(1, &mImmediateFence, /*waitAll=*/true, timeout));
}

} // namespace selwonk::vulkan
