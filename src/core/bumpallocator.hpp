#pragma once

#include <cstddef>
#include <type_traits>

namespace selwonk::core {

// Bump allocator for allocating memory in a first-fit manner
// Does not support individual deallocation
// Destructors will not be called on allocated memory
// Does not own its mapped data, user is responsible for freeing it manually
class BumpAllocator {
public:
  BumpAllocator(void* data, size_t capacity);

  void* allocate(size_t size);
  template <typename T> T* allocate() {
    static_assert(std::is_trivial<T>::value, "T must be trivial");
    return reinterpret_cast<T*>(allocate(sizeof(T)));
  }
  template <typename T> T* allocate(const T& value) {
    auto ptr = allocate<T>();
    *ptr = value;
    return ptr;
  }

  // Free all allocated memory
  void reset();

  // No copy
  BumpAllocator(const BumpAllocator&) = delete;
  BumpAllocator& operator=(const BumpAllocator&) = delete;

private:
  void* mData;
  size_t mCapacity;
  size_t mOffset = 0;
};
} // namespace selwonk::core
