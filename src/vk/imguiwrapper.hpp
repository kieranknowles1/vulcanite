#pragma once

#include <functional>

#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"
namespace selwonk::vulkan {
class ImguiWrapper {
public:
  void init(VulkanHandle &handle, SDL_Window *window);
  void destroy(VulkanHandle &handle);

  void draw(VulkanHandle &handle, vk::CommandBuffer cmd, vk::ImageView target);

  void submitImmediate(std::function<void(vk::CommandBuffer cmd)> &&function);

private:
  vk::Fence mFence;
  vk::CommandBuffer mBuffer;
  vk::CommandPool mPool;
  vk::DescriptorPool mDescriptorPool;
};
} // namespace selwonk::vulkan
