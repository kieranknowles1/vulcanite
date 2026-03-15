#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

#include <vncore/cvar.hpp>
#include <vncore/handle.hpp>

#include "buffer.hpp"
#include "shader.hpp"

namespace selwonk::vulkan {
// TODO: Add tag type, ref counting (probably using HandleList)
class BufferMap {
public:
  using Handle = core::Handle<BufferMap>;
  const static constexpr uint32_t Binding = 0;
  const static constexpr vk::DescriptorType DescriptorType =
      vk::DescriptorType::eStorageBuffer;

  void init(core::Cvar::Int& capacityVar);
  ~BufferMap();

  vk::DescriptorSetLayout getLayout() { return mLayout; }
  vk::DescriptorSet getSet() { return mSet; }

  Handle allocate(size_t size, Buffer::Usage usage);
  Buffer& getBuffer(Handle handle) { return mBuffers[handle.value()]; }

  template <typename T> Handle insert(std::span<T> data, Buffer::Usage usage) {
    return insertImpl(data.data(), data.size_bytes(), usage);
  }

  int size() { return mSize; }
  int getCapacity() { return mCapacity; }

private:
  void resize(int capacity);
  void writeDescriptor(Handle index, const Buffer& buffer);

  Handle insertImpl(void* data, size_t size, Buffer::Usage usage);

  Handle nextHandle();

  // TODO: Create BufferRef to allow reusing buffer objects
  std::vector<Buffer> mBuffers;

  vk::DescriptorSetLayout mLayout = nullptr;
  vk::DescriptorSet mSet;
  DescriptorAllocator mAllocator;
  std::vector<Handle::Backing> mFreelist;
  int mCapacity;
  int mSize = 0;
};
} // namespace selwonk::vulkan
