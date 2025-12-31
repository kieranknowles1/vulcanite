#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include "vulkaninit.hpp"

namespace selwonk::vulkan {
class Buffer {
public:
  // Create a temporary buffer for transferring data to the GPU.
  // TODO: Should we reuse transfer buffers? Should the transfers be async?
  static Buffer transferBuffer(VmaAllocator allocator, size_t size) {
    Buffer buf;
    buf.allocate(allocator, size, vk::BufferUsageFlagBits::eTransferSrc,
                 VMA_MEMORY_USAGE_CPU_TO_GPU);
    return buf;
  };

  // Allocate a buffer in VRAM for one of the following:
  // - VMA_MEMORY_USAGE_GPU_ONLY - Only read/written by the GPU. Use if
  // possible.
  // - VMA_MEMORY_USAGE_CPU_ONLY - CPU writes, GPU reads. Lives on main memory,
  // limited use cases.
  // - VMA_MEMORY_USAGE_CPU_TO_GPU - CPU writes, GPU reads. Lives on VRAM. May
  // not consume all VRAM if resizable BAR is disabled.
  // - VMA_MEMORY_USAGE_GPU_TO_CPU - GPU writes, CPU reads. Good for compute
  // shader output.
  void allocate(VmaAllocator allocator, size_t size,
                vk::BufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage);
  void free(VmaAllocator allocator);

  vk::DeviceAddress getDeviceAddress(vk::Device device) const;

  const vk::Buffer& getBuffer() const { return mBuffer; }
  const VmaAllocationInfo& getAllocationInfo() const { return mAllocationInfo; }

private:
  vk::Buffer mBuffer;
  VmaAllocation mAllocation;
  VmaAllocationInfo mAllocationInfo;
};

// A buffer that holds a single struct and is writable by the CPU. Intended
// for uniform buffers.
template <typename T> class StructBuffer {
public:
  void allocate(VmaAllocator allocator) {
    mBuffer.allocate(allocator, sizeof(T),
                     vk::BufferUsageFlagBits::eUniformBuffer,
                     VMA_MEMORY_USAGE_CPU_TO_GPU);
  }
  void free(VmaAllocator allocator) { mBuffer.free(allocator); }

  // For descriptor sets
  void write(vk::Device device, vk::DescriptorSet set) const {
    vk::DescriptorBufferInfo info = {
        .buffer = mBuffer.getBuffer(), .offset = 0, .range = sizeof(T)};
    auto write = VulkanInit::writeDescriptorSet(
        set, vk::DescriptorType::eUniformBuffer, 0, nullptr, &info);
    device.updateDescriptorSets(1, &write, 0, nullptr);
  }

  T* data() {
    return reinterpret_cast<T*>(mBuffer.getAllocationInfo().pMappedData);
  }

private:
  Buffer mBuffer;
};

} // namespace selwonk::vulkan
