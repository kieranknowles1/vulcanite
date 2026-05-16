#pragma once

#include <cstring>
#include <optional>
#include <stdexcept>
#include <string>
#include <vulkan/vulkan.hpp>

#include <vncore/cvar.hpp>
#include <vncore/handle.hpp>

#include "buffer.hpp"
#include "shader.hpp"
#include "vncore/handlelist.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"

namespace selwonk::vulkan {
// Array of fixed-size buffers, stored contiguously and referenced by index,
// such as material data. For variable sized buffers (i.e., an array of vertex
// arrays), use BufferMap
template <typename T> class BufferArray {
public:
  using Handle = core::HandleList<T>::Handle;
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

    capacityVar.addChangeCallback([this](int capacity) { resize(capacity); });
    capacityVar.addValidationCallback(
        [this](int capacity) -> std::optional<std::string> {
          if (capacity < mData.maxId()) {
            return std::make_optional("Cannot be smaller than max used ID (" +
                                      std::to_string(mData.maxId()) + ")");
          }
          return std::nullopt;
        });
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
    if (mData.size() >= mCapacity) {
      throw std::runtime_error("BufferArray full");
    }
    auto handle = mData.insert(data);

    T* gpuData = reinterpret_cast<T*>(mBuffer.getAllocationInfo().pMappedData) +
                 handle.value();
    *gpuData = data;
    return handle;
  }

  int size() { return mData.size(); }
  int capacity() { return mCapacity; }

  void incRef(Handle handle) { mData.incRef(handle); }

  // Decrement a handle's ref count. Return true if the handle was freed
  bool decRef(Handle handle) { return mData.decRef(handle); }

private:
  void resize(int capacity) {
    void* oldData = mBuffer.getAllocationInfo().pMappedData;
    // TODO: Delayed delete of buffer once current frame is done
    mBuffer.allocate(sizeof(T) * capacity, Buffer::Usage::FrameData,
                     "BufferArray");
    // TODO: Can't write descriptors while they're in use
    DescriptorAllocator::writeBuffer(mSet, DescriptorType, mBuffer.getBuffer(),
                                     0);

    if (oldData != nullptr) {
      memcpy(mBuffer.getAllocationInfo().pMappedData, oldData,
             sizeof(T) * std::min(capacity, mCapacity));
    }
    mCapacity = capacity;
  }

  core::HandleList<T> mData;

  // TODO: Deduplicate materials
  Buffer mBuffer;
  vk::DescriptorSetLayout mLayout;
  vk::DescriptorSet mSet;
  DescriptorAllocator mAllocator;
  int mCapacity;
};
} // namespace selwonk::vulkan
