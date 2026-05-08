#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"
#include <imgui.h>
namespace selwonk::vulkan {
class ImguiWrapper {
public:
  // TODO: Use IM_VEC2_CLASS_EXTRA for this
  static ImVec2 toImVec(vk::Extent3D extent) {
    return ImVec2(extent.width, extent.height);
  }

  void init(VulkanHandle& handle, SDL_Window* window);
  void destroy(VulkanHandle& handle);

  void draw(VulkanHandle& handle, vk::CommandBuffer cmd, vk::ImageView target);

private:
  vk::Fence mFence;
  vk::CommandBuffer mBuffer;
  vk::CommandPool mPool;
  vk::DescriptorPool mDescriptorPool;
};
} // namespace selwonk::vulkan
