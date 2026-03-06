#pragma once

#include "fastgltf/types.hpp"
#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <fastgltf/core.hpp>
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

  Image() = default;
  Image(vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags usage,
        bool mipmapped = false);
  ~Image();

  static Image load(const fastgltf::Asset& asset, const fastgltf::Image& image);

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

  vk::Image getImage() const { return mImage; }
  vk::ImageView getView() const { return mView; }
  vk::Format getFormat() const { return mFormat; }
  const vk::Extent3D& getExtent() const { return mExtent; }

  // No copy
  Image(const Image&) = delete;
  Image& operator=(const Image&) = delete;
  Image(Image&& other) { fillFrom(other); };
  Image& operator=(Image&& other) {
    fillFrom(other);
    return *this;
  }

private:
  void fillFrom(Image& other) {
    mImage = other.mImage;
    other.mImage = nullptr;
    mView = other.mView;
    other.mView = nullptr;
    mAllocation = other.mAllocation;
    other.mAllocation = nullptr;
    mExtent = other.mExtent;
    mFormat = other.mFormat;
  }

  struct ImgData {
    uint32_t width;
    uint32_t height;
    const unsigned char* data;
  };
  static ImgData visitDataSrc(const fastgltf::Asset& asset,
                              const fastgltf::DataSource& data);
  static ImgData loadFromMemory(const std::byte* bytes, int size);

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
