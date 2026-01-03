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

  void* allocate(size_t size);
  template <typename T> T* allocate() {
    static_assert(std::is_trivial<T>::value, "T must be trivial");
    return reinterpret_cast<T*>(allocate(sizeof(T)));
  }
  template <typename T> T* allocate(const T& value) {
    T* ptr = allocate<T>();
    *ptr = value;
    return ptr;
  }

  // Free all allocated memory
  void reset();

  // No copy
  BumpAllocator(const BumpAllocator&) = delete;
  BumpAllocator& operator=(const BumpAllocator&) = delete;

private:
  Buffer mBuffer;
  size_t mOffset;
};
} // namespace selwonk::vulkan
