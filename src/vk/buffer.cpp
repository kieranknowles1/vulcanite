#include "buffer.hpp"

#include "utility.hpp"
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_core.h>

namespace selwonk::vulkan {
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
}

vk::DeviceAddress Buffer::getDeviceAddress(vk::Device device) const {
  vk::BufferDeviceAddressInfo addrInfo = {.buffer = mBuffer};
  return device.getBufferAddress(&addrInfo);
}

void Buffer::free(VmaAllocator allocator) {
  vmaDestroyBuffer(allocator, *vkUnwrap(mBuffer), mAllocation);
}
} // namespace selwonk::vulkan
