#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

#include "buffer.hpp"
#include "handle.hpp"
#include "shader.hpp"

namespace selwonk::vulkan {
// TODO: Replace ResourceMap with this
class BufferMap {
public:
  using Handle = Handle;
  const static constexpr uint32_t Binding = 0;
  const static constexpr vk::DescriptorType DescriptorType =
      vk::DescriptorType::eStorageBuffer;

  void init(size_t capacity);
  ~BufferMap();

  vk::DescriptorSetLayout getLayout() { return mLayout; }
  vk::DescriptorSet getSet() { return mSet; }

  Handle allocate(size_t size, Buffer::Usage usage);
  Buffer& getBuffer(Handle handle) { return mBuffers[handle.value()]; }

  template <typename T> Handle insert(std::span<T> data, Buffer::Usage usage) {
    return insertImpl(data.data(), data.size_bytes(), usage);
  }

private:
  Handle insertImpl(void* data, size_t size, Buffer::Usage usage);

  Handle nextHandle();

  // TODO: Create BufferRef to allow reusing buffer objects
  std::vector<Buffer> mBuffers;

  vk::DescriptorSetLayout mLayout;
  vk::DescriptorSet mSet;
  DescriptorAllocator mAllocator;
  std::vector<Handle::Backing> mFreelist;
};
} // namespace selwonk::vulkan
