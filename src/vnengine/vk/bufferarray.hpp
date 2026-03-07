#pragma once

#include <stdexcept>
#include <vulkan/vulkan.hpp>

#include <vncore/cvar.hpp>

#include "buffer.hpp"
#include "handle.hpp"
#include "shader.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"

namespace selwonk::vulkan {
// Array of fixed-size buffers, stored contiguously and referenced by index
template <typename T> class BufferArray {
public:
  using Handle = Handle;
  const static constexpr uint32_t Binding = 0;
  const static constexpr vk::DescriptorType DescriptorType =
      vk::DescriptorType::eStorageBuffer;

  void init(core::Cvar::Int& capacityVar) {
    DescriptorLayoutBuilder builder;
    builder.addBinding(0, DescriptorType);
    mLayout = builder.build(VulkanHandle::get().mDevice,
                            vk::ShaderStageFlagBits::eVertex |
                                vk::ShaderStageFlagBits::eFragment);

    std::array<DescriptorAllocator::PoolSizeRatio, 1> poolSize = {{{
        .type = DescriptorType,
        .ratio = 1.0f,
    }}};
    mAllocator.init(1, poolSize);
    mSet = mAllocator.allocate(mLayout);

    resize(capacityVar.value());
  }
  ~BufferArray() {
    auto& handle = VulkanHandle::get();
    mAllocator.destroy();
    handle.mDevice.destroyDescriptorSetLayout(mLayout, nullptr);
    mBuffer.free(handle.mAllocator);
  }

  vk::DescriptorSetLayout getLayout() { return mLayout; }
  vk::DescriptorSet getSet() { return mSet; }

  Handle insert(const T& data) {
    if (mSize >= mCapacity) {
      throw std::runtime_error("BufferArray full");
    }
    T* gpuData =
        reinterpret_cast<T*>(mBuffer.getAllocationInfo().pMappedData) + mSize;
    *gpuData = data;
    Handle handle(mSize);
    mSize++;
    return handle;
  }

  int size() { return mSize; }
  int capacity() { return mCapacity; }

private:
  void resize(int capacity) {
    // TODO: Delayed delete of buffer once current frame is done
    mBuffer.allocate(sizeof(T) * capacity, Buffer::Usage::FrameData);
    DescriptorAllocator::writeBuffer(mSet, DescriptorType, mBuffer.getBuffer(),
                                     0);
    mCapacity = capacity;
  }

  // TODO: Deduplicate materials
  Buffer mBuffer;
  vk::DescriptorSetLayout mLayout;
  vk::DescriptorSet mSet;
  DescriptorAllocator mAllocator;
  int mCapacity;
  int mSize = 0;
};
} // namespace selwonk::vulkan
