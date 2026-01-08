#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include "vulkan/vulkan.hpp"
#include "vulkaninit.hpp"

namespace selwonk::vulkan {
class Buffer {
public:
  // Allocation that can be read by both the GPU and CPU
  template <typename T> struct CrossAllocation {
    T *cpu = 0;
    vk::DeviceAddress gpu = 0;

    static CrossAllocation<T> from(CrossAllocation<void> untyped) {
      return {reinterpret_cast<T *>(untyped.cpu), untyped.gpu};
    }
  };

  enum class Usage {
    // Bindless vertexes/indexes, read by shaders rather than fixed-function
    BindlessVertex,
    BindlessIndex,

    // Temporary buffer for copy to the GPU
    Transfer,
  };

  struct VulkanBufferUsage {
    vk::BufferUsageFlags bufUse;
    VmaMemoryUsage memUse;

    explicit VulkanBufferUsage(Usage usage);
  };

  // TODO: Meshes etc shouldn't need to know about vulkan, abstract that bit away if we want to do a webgpu port
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
  void allocate(size_t size, Usage usage);

  void allocate(VmaAllocator allocator, size_t size,
                vk::BufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage);
  void free(VmaAllocator allocator);

  vk::DeviceAddress getDeviceAddress() const { return mDeviceAddress; }

  const vk::Buffer &getBuffer() const { return mBuffer; }
  const VmaAllocationInfo &getAllocationInfo() const { return mAllocationInfo; }

  // Upload data to the GPU from the CPU
  template <typename T> void uploadToGpu(std::span<char> data) {
    uploadToGpu(data.data(), data.size_bytes());
  }
  void uploadToGpu(void *data, size_t size);

private:
  vk::Buffer mBuffer;
  vk::DeviceAddress mDeviceAddress;
  VmaAllocation mAllocation;
  VmaAllocationInfo mAllocationInfo;
};

// A buffer that holds a single struct and is writable by the CPU. Intended
// for uniform buffers.
// TODO: A bump allocator may be better here.
template <typename T> class StructBuffer {
public:
  void allocate(
      VmaAllocator allocator,
      vk::BufferUsageFlagBits usage = vk::BufferUsageFlagBits::eUniformBuffer) {
    mBuffer.allocate(allocator, sizeof(T), usage, VMA_MEMORY_USAGE_CPU_TO_GPU);
  }
  void free(VmaAllocator allocator) { mBuffer.free(allocator); }
  vk::DeviceAddress getDeviceAddress() { return mBuffer.getDeviceAddress(); }

  // For descriptor sets
  void write(vk::Device device, vk::DescriptorSet set) const {
    vk::DescriptorBufferInfo info = {
        .buffer = mBuffer.getBuffer(), .offset = 0, .range = sizeof(T)};
    auto write = VulkanInit::writeDescriptorSet(
        set, vk::DescriptorType::eUniformBuffer, 0, nullptr, &info);
    device.updateDescriptorSets(1, &write, 0, nullptr);
  }

  T *data() {
    return reinterpret_cast<T *>(mBuffer.getAllocationInfo().pMappedData);
  }

private:
  Buffer mBuffer;
};

} // namespace selwonk::vulkan
