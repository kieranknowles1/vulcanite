#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace selwonk::vulkan {
class VulkanHandle;

class Image {
public:
  static constexpr size_t bytesPerPixel(vk::Format format) {
    using enum vk::Format;
    switch (format) {
    case eR8G8B8A8Unorm:
      return 4;
    default:
      assert(false && "Unsupported format");
    }
  }

  void init(VulkanHandle &handle, vk::Extent3D extent, vk::Format format,
            vk::ImageUsageFlags usage, bool mipmapped = false);
  void fill(std::span<const unsigned char> data);
  template <typename T> void fill(std::span<const T> data) {
    auto size = data.size() * sizeof(T);
    fill(std::span<const unsigned char>(
        reinterpret_cast<const unsigned char *>(data.data()), size));
  }
  template <typename T, size_t S> void fill(const std::array<T, S> &data) {
    auto size = data.size() * sizeof(T);
    fill(std::span<const unsigned char>(
        reinterpret_cast<const unsigned char *>(data.data()), size));
  }
  void fill(const void *data, size_t size) {
    fill(std::span<const unsigned char>(
        static_cast<const unsigned char *>(data), size));
  }
  void destroy(VulkanHandle &handle);

  void copyFromImage(vk::CommandBuffer cmd, const Image &source);
  static void copyToSwapchainImage(vk::CommandBuffer cmd, Image source,
                                   vk::Image destination, vk::Extent3D extent);

  vk::Image getImage() { return mImage; }
  vk::ImageView getView() { return mView; }
  vk::Format getFormat() { return mFormat; }

private:
  vk::Image mImage = nullptr;
  vk::ImageView mView = nullptr;
  VmaAllocation mAllocation = nullptr;
  vk::Extent3D mExtent = {};
  vk::Format mFormat = vk::Format::eUndefined;
};
} // namespace selwonk::vulkan
