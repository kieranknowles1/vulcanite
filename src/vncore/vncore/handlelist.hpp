#pragma once

#include "chunkedarray.hpp"
#include "handle.hpp"
#include <cassert>
#include <cstdint>
#include <spdlog/spdlog.h>
#include <vector>

namespace selwonk::core {
// Indexed and ref counted container for an object referenced by handle
// Handles must be manually incremented/decremented
// TODO: Use this for resourcemap/texturemanager/meshes
template <typename T, typename THandle = core::Handle<T>,
          size_t ChunkSize = 1024>
class HandleList {
public:
  using Handle = THandle;
  ~HandleList() {
#ifndef NDEBUG
    // FIXME: This iterates over unused slots
    for (size_t i = 0; i < mSlots.capacity(); i++) {
      if (mSlots[i].mRefCount > 0) {
        spdlog::warn("Slot {} leaked with {} refs", i, mSlots[i].mRefCount);
      }
    }
#endif
  }

  // Insert a new handle, which starts with 1 reference
  template <typename... Args> Handle insert(Args&&... args) {
    auto index = nextIndex();
    auto& slot = mSlots[index];
    Handle handle(index, slot.mGeneration);
    new (slot.mStorage) T(std::forward<Args>(args)...);
    incRef(handle);
    return handle;
  }

  T& get(Handle handle) {
    auto& slot = mSlots[handle.value()];
    generationCheck(handle);
    return *slot.ptr();
  }

  void incRef(Handle handle) {
    auto& slot = mSlots[handle.value()];
    generationCheck(handle);
    slot.mRefCount++;
  }
  // Decrement a handle's ref count. Return true if the handle was freed
  bool decRef(Handle handle) {
    auto& slot = mSlots[handle.value()];
    generationCheck(handle);
    assert(slot.mRefCount > 0 && "Attempted to free an empty slot");

    slot.mRefCount--;

    if (slot.mRefCount <= 0) {
      // Call destructor manually
      slot.ptr()->~T();
      slot.mGeneration++;
      mFreeList.push_back(handle.value());
    }
    return slot.mRefCount <= 0;
  }
  size_t refCount(Handle handle) {
    generationCheck(handle);
    return mSlots[handle.value()].mRefCount;
  }

  size_t maxId() { return mNextIndex; }
  size_t size() { return mNextIndex - mFreeList.size(); }

private:
  void generationCheck(Handle handle) {
#ifndef NDEBUG
    auto& slot = mSlots[handle.value()];
    assert(slot.mGeneration == handle.generation() && "Generation mismatch");
#endif
  }

  Handle::Backing nextIndex() {
    if (!mFreeList.empty()) {
      auto next = mFreeList.back();
      mFreeList.pop_back();
      return next;
    }
    auto index = mNextIndex;
    mNextIndex++;
    return index;
  }

  struct Slot {
    // Array of data to store a T in with placement new
    alignas(T) char mStorage[sizeof(T)];
    Handle::GenerationBacking mGeneration;
    uint32_t mRefCount;

    T* ptr() { return reinterpret_cast<T*>(mStorage); }
  };

  ChunkedArray<Slot> mSlots;
  size_t mNextIndex = 0;
  std::vector<typename Handle::Backing> mFreeList;
};
} // namespace selwonk::core
