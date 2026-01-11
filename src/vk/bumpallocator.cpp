#include "bumpallocator.hpp"
#include "buffer.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanhandle.hpp"

namespace selwonk::vulkan {

BumpAllocator::BumpAllocator(size_t capacity, vk::BufferUsageFlagBits usage) {
  mBuffer.allocate(VulkanHandle::get().mAllocator, capacity, usage,
                   VMA_MEMORY_USAGE_CPU_TO_GPU);
}

BumpAllocator::~BumpAllocator() {
  mBuffer.free(VulkanHandle::get().mAllocator);
}

Buffer::CrossAllocation<void> BumpAllocator::allocate(size_t size) {
  if (mOffset + size > mBuffer.getAllocationInfo().size) {
    throw std::bad_alloc();
  }
  void* ptr = (char*)mBuffer.getAllocationInfo().pMappedData + mOffset;
  vk::DeviceAddress device = mBuffer.getDeviceAddress() + mOffset;
  mOffset += size;
  return Buffer::CrossAllocation<void>{ptr, device};
}

void BumpAllocator::reset() { mOffset = 0; }

} // namespace selwonk::vulkan
