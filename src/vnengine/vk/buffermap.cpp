#include "buffermap.hpp"

#include "vulkan/vulkan.hpp"
#include <stdexcept>
#include <vnvulkan/vulkanhandle.hpp>

namespace selwonk::vulkan {

void BufferMap::init(core::Cvar::Int& capacityVar) {
  resize(capacityVar.value());

  capacityVar.addChangeCallback([this](int capacity) { resize(capacity); });
  capacityVar.addValidationCallback(
      [this](int capacity) -> std::optional<std::string> {
        if (capacity < mData.maxId()) {
          return std::make_optional(
              "Cannot be smaller than number max allocated ID (" +
              std::to_string(mData.maxId()) + ")");
        }
        return std::nullopt;
      });
}

void BufferMap::resize(int capacity) {
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

BufferMap::~BufferMap() {
  auto& handle = VulkanHandle::get();
  handle.mDevice.destroyDescriptorSetLayout(mLayout, nullptr);
  mAllocator.destroy();
}

BufferMap::Handle BufferMap::allocate(size_t size, Buffer::Usage usage,
                                      const char* name) {
  if (mData.maxId() >= mCapacity) {
    throw std::runtime_error("BufferMap full");
  }

  auto handle = mData.insert();
  auto& buffer = mData.get(handle);
  buffer.allocate(size, usage, name);

  writeDescriptor(handle, buffer);

  return handle;
}

void BufferMap::writeDescriptor(Handle index, const Buffer& buffer) {
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

BufferMap::Handle BufferMap::insertImpl(void* data, size_t size,
                                        Buffer::Usage usage) {
  auto handle = allocate(size, usage);
  auto& buffer = getBuffer(handle);
  buffer.uploadToGpu(data, size);
  return handle;
}

} // namespace selwonk::vulkan
