#pragma once

#include <type_traits>

#include "buffer.hpp"

namespace selwonk::vulkan {

// Bump allocator for allocating memory in a first-fit manner
// Does not support individual deallocation
// Destructors will not be called on allocated memory
// TODO: Make this generic, shouldn't be vulkan specific
class BumpAllocator {
public:
  BumpAllocator(Buffer& buffer);

  Buffer::CrossAllocation<void> allocate(size_t size);
  template <typename T> Buffer::CrossAllocation<T> allocate() {
    static_assert(std::is_trivial<T>::value, "T must be trivial");
    return Buffer::CrossAllocation<T>::from(allocate(sizeof(T)));
  }
  template <typename T> Buffer::CrossAllocation<T> allocate(const T& value) {
    auto ptr = allocate<T>();
    *(ptr.cpu) = value;
    return ptr;
  }

  // Free all allocated memory
  void reset();

  // No copy
  BumpAllocator(const BumpAllocator&) = delete;
  BumpAllocator& operator=(const BumpAllocator&) = delete;

private:
  void* mData;
  vk::DeviceAddress mDeviceAddress;
  size_t mCapacity;
  size_t mOffset = 0;
};
} // namespace selwonk::vulkan
