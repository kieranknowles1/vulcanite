#pragma once

#include <vulkan/vulkan.hpp>

#include <vncore/cvar.hpp>
#include <vncore/handle.hpp>
#include <vncore/handlelist.hpp>

#include "buffer.hpp"
#include "shader.hpp"
#include "vulkanhandle.hpp"

namespace selwonk::vulkan {

// List of buffers each containing an array of a primitive, such as vertex
// buffers. For fixed-sized buffers (i.e., mat data) use BufferArray
template <typename Handle> class BufferMap {
public:
  const static constexpr uint32_t Binding = 0;
  const static constexpr vk::DescriptorType DescriptorType =
      vk::DescriptorType::eStorageBuffer;

  // TODO: RAII
  void init(core::Cvar::Int& capacityVar) {
    resize(capacityVar.value());

    capacityVar.getStore().addChange(
        [this](int capacity) { resize(capacity); });
    capacityVar.getStore().addValidate(
        [this](int capacity) -> std::optional<std::string> {
          if (capacity < mData.maxId()) {
            return std::make_optional(
                "Cannot be smaller than number max allocated ID (" +
                std::to_string(mData.maxId()) + ")");
          }
          return std::nullopt;
        });
  }
  ~BufferMap() {
    auto& handle = VulkanHandle::get();
    handle.mDevice.destroyDescriptorSetLayout(mLayout, nullptr);
    mAllocator.destroy();
  }

  vk::DescriptorSetLayout getLayout() { return mLayout; }
  vk::DescriptorSet getSet() { return mSet; }

  Handle allocate(size_t size, Buffer::Usage usage,
                  const char* name = nullptr) {
    if (mData.maxId() >= mCapacity) {
      throw std::runtime_error("BufferMap full");
    }

    auto handle = mData.insert();
    auto& buffer = mData.get(handle);
    buffer.allocate(size, usage, name);

    writeDescriptor(handle, buffer);

    return handle;
  }
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
  void resize(int capacity) {
    // TODO: Delayed delete of descriptor layout and allocator once current
    // frame is done
    mCapacity = capacity;

    DescriptorLayoutBuilder builder;
    builder.addBinding(Binding, DescriptorType, capacity);
    mLayout = builder.build(VulkanHandle::get().mDevice,
                            vk::ShaderStageFlagBits::eVertex);
    std::array<DescriptorAllocator::PoolSizeRatio, 1> ratios = {
        {{DescriptorType, 1}}};
    mAllocator.init(capacity, ratios);

    mSet = mAllocator.allocate(mLayout);
  }
  void writeDescriptor(Handle index, const Buffer& buffer) {
    vk::DescriptorBufferInfo info = {
        .buffer = buffer.getBuffer(),
        .offset = 0,
        .range = buffer.getSize(),
    };
    vk::WriteDescriptorSet write = {
        .dstSet = mSet,
        .dstBinding = Binding,
        .dstArrayElement = index.value(),
        .descriptorCount = 1,
        .descriptorType = DescriptorType,
        .pBufferInfo = &info,
    };
    assert(mSet != nullptr);
    assert(buffer.getBuffer() != nullptr);
    VulkanHandle::get().mDevice.updateDescriptorSets(1, &write, 0, nullptr);
  }

  Handle insertImpl(void* data, size_t size, Buffer::Usage usage) {
    auto handle = allocate(size, usage);
    auto& buffer = getBuffer(handle);
    buffer.uploadToGpu(data, size);
    return handle;
  }

  core::HandleList<Buffer, Handle> mData;

  vk::DescriptorSetLayout mLayout = nullptr;
  vk::DescriptorSet mSet;
  DescriptorAllocator mAllocator;
  int mCapacity;
};
} // namespace selwonk::vulkan
