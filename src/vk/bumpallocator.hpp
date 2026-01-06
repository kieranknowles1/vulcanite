#pragma once

#include "buffer.hpp"
#include <type_traits>
namespace selwonk::vulkan {

// Bump allocator for allocating memory in a first-fit manner
// Does not support individual deallocation
// Destructors will not be called on allocated memory
class BumpAllocator {
public:
  BumpAllocator(size_t capacity, vk::BufferUsageFlagBits usage);
  ~BumpAllocator();

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
  Buffer mBuffer;
  size_t mOffset = 0;
};
} // namespace selwonk::vulkan
