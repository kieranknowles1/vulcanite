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

  Image(vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags usage,
        bool mipmapped = false);
  ~Image();

  void fill(std::span<const unsigned char> data);
  template <typename T> void fill(std::span<const T> data) {
    auto size = data.size() * sizeof(T);
    fill(std::span<const unsigned char>(
        reinterpret_cast<const unsigned char*>(data.data()), size));
  }
  template <typename T, size_t S> void fill(const std::array<T, S>& data) {
    auto size = data.size() * sizeof(T);
    fill(std::span<const unsigned char>(
        reinterpret_cast<const unsigned char*>(data.data()), size));
  }
  void fill(const void* data, size_t size) {
    fill(std::span<const unsigned char>(static_cast<const unsigned char*>(data),
                                        size));
  }

  void copyFromImage(vk::CommandBuffer cmd, const Image& source);
  static void copyToSwapchainImage(vk::CommandBuffer cmd, const Image& source,
                                   vk::Image destination, vk::Extent3D extent);

  vk::Image getImage() { return mImage; }
  vk::ImageView getView() { return mView; }
  vk::Format getFormat() { return mFormat; }

  // No copy/move
  Image(const Image&) = delete;
  Image& operator=(const Image&) = delete;
  Image(Image&&) = delete;
  Image& operator=(Image&&) = delete;

private:
  static void copyImpl(vk::CommandBuffer cmd, vk::Image source,
                       vk::Extent3D srcExtent, vk::Image destination,
                       vk::Extent3D dstExtent);

  vk::Image mImage = nullptr;
  vk::ImageView mView = nullptr;
  VmaAllocation mAllocation = nullptr;
  vk::Extent3D mExtent = {};
  vk::Format mFormat = vk::Format::eUndefined;
};
} // namespace selwonk::vulkan
