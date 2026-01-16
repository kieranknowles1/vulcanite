#include "buffer.hpp"

#include "utility.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"
#include <vulkan/vulkan_core.h>

namespace selwonk::vulkan {
Buffer::VulkanBufferUsage::VulkanBufferUsage(Usage usage) {
  using enum Usage;
  switch (usage) {
  case BindlessVertex:
  case BindlessIndex:
    // TODO: Don't need device address here or in required features
    bufUse = vk::BufferUsageFlagBits::eShaderDeviceAddress |
             vk::BufferUsageFlagBits::eStorageBuffer |
             vk::BufferUsageFlagBits::eTransferDst;
    memUse = VMA_MEMORY_USAGE_GPU_ONLY;
    return;
  case BindlessMaterial:
    // TODO: Don't need device address here or in required features
    bufUse = vk::BufferUsageFlagBits::eShaderDeviceAddress;
    memUse = VMA_MEMORY_USAGE_CPU_TO_GPU;
    return;
  case Transfer:
    bufUse = vk::BufferUsageFlagBits::eTransferSrc;
    memUse = VMA_MEMORY_USAGE_CPU_TO_GPU;
    return;
  case DebugLines:
    bufUse = vk::BufferUsageFlagBits::eStorageBuffer |
             vk::BufferUsageFlagBits::eShaderDeviceAddress;
    memUse = VMA_MEMORY_USAGE_CPU_TO_GPU;
    return;
  }
}

void Buffer::uploadToGpu(void* data, size_t size) {
  assert(size <= mAllocationInfo.size && "Buffer overrun");
  auto& handle = VulkanHandle::get();
  // TODO: Should we reuse the staging buffer?
  // TODO: Should transfers be async?
  Buffer stagingBuffer;
  stagingBuffer.allocate(size, Usage::Transfer);

  void* gpuData = stagingBuffer.getAllocationInfo().pMappedData;
  assert(gpuData != nullptr);
  memcpy(gpuData, data, size);

  handle.immediateSubmit([&](vk::CommandBuffer cmd) {
    vk::BufferCopy copy = {.srcOffset = 0, .dstOffset = 0, .size = size};
    cmd.copyBuffer(stagingBuffer.getBuffer(), mBuffer, 1, &copy);
  });
  stagingBuffer.free(handle.mAllocator);
}

void Buffer::allocate(size_t size, Usage usage) {
  auto use = VulkanBufferUsage(usage);
  return allocate(VulkanHandle::get().mAllocator, size, use.bufUse, use.memUse);
}

void Buffer::allocate(VmaAllocator allocator, size_t size,
                      vk::BufferUsageFlags bufferUsage,
                      VmaMemoryUsage memoryUsage) {
  vk::BufferCreateInfo createInfo = {
      .size = size,
      .usage = bufferUsage,
  };
  VmaAllocationCreateInfo allocInfo = {
      // Always create a buffer with a mapped memory if possible,
      // mapping will be nullptr on platforms that do not support it unless
      // created with a usage flag that supports it, or on a platform with
      // unified memory
      .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
      .usage = memoryUsage,
  };

  check(vmaCreateBuffer(allocator, vkUnwrap(createInfo), &allocInfo,
                        vkUnwrap(mBuffer), &mAllocation, &mAllocationInfo));
  vk::BufferDeviceAddressInfo addrInfo = {.buffer = mBuffer};
  if (bufferUsage & vk::BufferUsageFlagBits::eShaderDeviceAddress)
    mDeviceAddress = VulkanHandle::get().mDevice.getBufferAddress(&addrInfo);
}

void Buffer::free(VmaAllocator allocator) {
  vmaDestroyBuffer(allocator, *vkUnwrap(mBuffer), mAllocation);
}
} // namespace selwonk::vulkan
