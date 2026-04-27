#pragma once

#include <vulkan/vulkan.hpp>

#include <vncore/cvar.hpp>
#include <vncore/handle.hpp>
#include <vncore/handlelist.hpp>

#include "buffer.hpp"
#include "shader.hpp"
#include "vulkanhandle.hpp"

namespace selwonk::vulkan {
// TODO: Add tag type

// List of buffers each containing an array of a primitive, such as vertex
// buffers. For fixed-sized buffers (i.e., mat data) use BufferArray
class BufferMap {
public:
  using Handle = core::HandleList<Buffer>::Handle;
  const static constexpr uint32_t Binding = 0;
  const static constexpr vk::DescriptorType DescriptorType =
      vk::DescriptorType::eStorageBuffer;

  void init(core::Cvar::Int& capacityVar);
  ~BufferMap();

  vk::DescriptorSetLayout getLayout() { return mLayout; }
  vk::DescriptorSet getSet() { return mSet; }

  Handle allocate(size_t size, Buffer::Usage usage, const char* name = nullptr);
  Buffer& getBuffer(Handle handle) { return mData.get(handle); }

  template <typename T> Handle insert(std::span<T> data, Buffer::Usage usage) {
    return insertImpl(data.data(), data.size_bytes(), usage);
  }

  int size() { return mData.size(); }
  int getCapacity() { return mCapacity; }

  void incRef(Handle handle) { mData.incRef(handle); }
  void decRef(Handle handle) {
    auto count = mData.refCount(handle);
    // We're freeing the last ref
    if (count <= 1) {
      mData.get(handle).free(VulkanHandle::get().mAllocator);
    }
    mData.decRef(handle);
  }

private:
  void resize(int capacity);
  void writeDescriptor(Handle index, const Buffer& buffer);

  Handle insertImpl(void* data, size_t size, Buffer::Usage usage);

  core::HandleList<Buffer> mData;

  vk::DescriptorSetLayout mLayout = nullptr;
  vk::DescriptorSet mSet;
  DescriptorAllocator mAllocator;
  int mCapacity;
};
} // namespace selwonk::vulkan
