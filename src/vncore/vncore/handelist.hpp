#pragma once

#include "vncore/chunkedarray.hpp"
#include <cassert>
#include <cstdint>
#include <fmt/base.h>
#include <limits>
#include <vector>

namespace selwonk::core {
// Indexed and ref counted container for an object referenced by handle
// Handles must be manually incremented/decremented
// TODO: Use this for resourcemap/samplercache/texturemanager/meshes
template <typename T, size_t ChunkSize = 1024> class HandleList {
public:
  class Handle {
  public:
    // Layout:
    // i: index
    // g: generation
    // gggggggg-iiiiiiii-iiiiiiii-iiiiiiii
    // All ones represents an invalid handle
    using Backing = uint32_t;
    using GenerationBacking = uint8_t;

    const static constexpr int IndexBits = 24;
    const static constexpr int GenerationBits =
        std::numeric_limits<Backing>::digits - IndexBits;
    static_assert(GenerationBits <=
                      std::numeric_limits<GenerationBacking>::digits,
                  "GenerationBacking cannot hold generation number");

    const static constexpr Backing InvalidValue =
        std::numeric_limits<Backing>::max();

    const static constexpr Backing GenerationMask = InvalidValue << IndexBits;
    const static constexpr Backing IndexMask = InvalidValue >> GenerationBits;

    static_assert((GenerationMask & IndexMask) == 0,
                  "Generation and index overlap");
    static_assert((GenerationMask | IndexMask) == InvalidValue,
                  "Unused bits in handle");

    explicit Handle(Backing v, GenerationBacking gen)
        : mValue(v | (gen << IndexBits)) {
      assert(generation() == gen);
      assert(value() == v);
    }
    Handle() : mValue(InvalidValue) {}
    constexpr Backing value() const {
      assert(valid());
      return mValue & IndexMask;
    }
    constexpr GenerationBacking generation() const {
      assert(valid());
      return (mValue & GenerationMask) >> IndexBits;
    }
    constexpr bool valid() const { return mValue != InvalidValue; }

  private:
    Backing mValue;
  };

  ~HandleList() {
#ifndef NDEBUG
    // FIXME: This iterates over unused slots
    for (size_t i = 0; i < mSlots.capacity(); i++) {
      if (mSlots[i].mRefCount > 0) {
        fmt::println("Slot {} leaked with {} refs", i, mSlots[i].mRefCount);
      }
    }
#endif
  }

  // Insert a new handle, which starts with 1 reference
  template <typename... Args> Handle insert(Args... args) {
    auto index = nextIndex();
    auto& slot = mSlots[index];
    Handle handle(index, slot.mGeneration);
    new (slot.mStorage) T(args...);
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
  void decRef(Handle handle) {
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
  }

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
    uint32_t mRefCount;
    Handle::GenerationBacking mGeneration;

    T* ptr() { return reinterpret_cast<T*>(mStorage); }
  };

  ChunkedArray<Slot> mSlots;
  size_t mNextIndex = 0;
  std::vector<typename Handle::Backing> mFreeList;
};
} // namespace selwonk::core
