#include "bumpallocator.hpp"

#include <new>

namespace selwonk::core {

BumpAllocator::BumpAllocator(void* data, size_t capacity) {
  mData = data;
  mCapacity = capacity;
}

void* BumpAllocator::allocate(size_t size) {
  if (mOffset + size > mCapacity) {
    throw std::bad_alloc();
  }
  void* ptr = (char*)mData + mOffset;
  mOffset += size;
  return ptr;
}

void BumpAllocator::reset() { mOffset = 0; }

} // namespace selwonk::core
