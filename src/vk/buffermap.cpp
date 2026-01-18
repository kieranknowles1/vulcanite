#include "buffermap.hpp"
#include "handle.hpp"
#include "shader.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"

namespace selwonk::vulkan {

void BufferMap::init(core::Cvar::Int& capacityVar) {
  resize(capacityVar.value());

  capacityVar.addChangeCallback([this](int capacity) { resize(capacity); });
  capacityVar.addValidationCallback(
      [this](int capacity) -> std::optional<std::string> {
        if (capacity < mBuffers.size()) {
          return std::make_optional(
              "Capacity cannot be smaller number of allocated buffers (" +
              std::to_string(mBuffers.size()) + ")");
        }
        return std::nullopt;
      });
}

void BufferMap::resize(int capacity) {
  // TODO: Delayed delete of descriptor layout and allocator once current
  // frame is done

  DescriptorLayoutBuilder builder;
  builder.addBinding(Binding, DescriptorType, capacity);
  mLayout = builder.build(VulkanHandle::get().mDevice,
                          vk::ShaderStageFlagBits::eVertex);
  std::array<DescriptorAllocator::PoolSizeRatio, 1> ratios = {
      {{DescriptorType, 1}}};
  mAllocator.init(capacity, ratios);

  mSet = mAllocator.allocateImpl(mLayout);

  for (int i = 0; i < mBuffers.size(); i++) {
    writeDescriptor(Handle(i), mBuffers[i]);
  }
}

BufferMap::~BufferMap() {
  auto& handle = VulkanHandle::get();
  handle.mDevice.destroyDescriptorSetLayout(mLayout, nullptr);
  mAllocator.destroy();

  for (auto& buffer : mBuffers) {
    buffer.free(handle.mAllocator);
  }
}

Handle BufferMap::allocate(size_t size, Buffer::Usage usage) {
  auto handle = nextHandle();
  auto& buffer = mBuffers[handle.value()];
  buffer.allocate(size, usage);

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
  VulkanHandle::get().mDevice.updateDescriptorSets(1, &write, 0, nullptr);
}

Handle BufferMap::insertImpl(void* data, size_t size, Buffer::Usage usage) {
  auto handle = allocate(size, usage);
  auto& buffer = getBuffer(handle);
  buffer.uploadToGpu(data, size);
  return handle;
}

Handle BufferMap::nextHandle() {
  if (!mFreelist.empty()) {
    auto top = mFreelist.back();
    mFreelist.pop_back();
    return Handle(top);
  }
  mBuffers.resize(mBuffers.size() + 1);
  return Handle(mBuffers.size() - 1);
}

} // namespace selwonk::vulkan
