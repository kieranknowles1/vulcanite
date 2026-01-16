#include "buffer.hpp"
#include "bumpallocator.hpp"
#include "vulkan/vulkan.hpp"

namespace selwonk::vulkan {

BumpAllocator::BumpAllocator(Buffer& buffer) {
  mData = buffer.getAllocationInfo().pMappedData;
  mDeviceAddress = buffer.getDeviceAddress();
  mCapacity = buffer.getAllocationInfo().size;
}

Buffer::CrossAllocation<void> BumpAllocator::allocate(size_t size) {
  if (mOffset + size > mCapacity) {
    throw std::bad_alloc();
  }
  void* ptr = (char*)mData + mOffset;
  vk::DeviceAddress device = mDeviceAddress + mOffset;
  mOffset += size;
  return Buffer::CrossAllocation<void>{ptr, device};
}

void BumpAllocator::reset() { mOffset = 0; }

} // namespace selwonk::vulkan
