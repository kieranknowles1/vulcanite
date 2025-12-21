#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"
namespace selwonk::vulkan {
class ImguiWrapper {
public:
  void init(VulkanHandle &handle, SDL_Window *window);
  void destroy(VulkanHandle &handle);

  void draw(VulkanHandle &handle, vk::CommandBuffer cmd, vk::ImageView target);

private:
  vk::Fence mFence;
  vk::CommandBuffer mBuffer;
  vk::CommandPool mPool;
  vk::DescriptorPool mDescriptorPool;
};
} // namespace selwonk::vulkan
