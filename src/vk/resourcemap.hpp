#pragma once

#include "handle.hpp"
#include <cstdint>
#include <limits>
#include <map>
#include <vector>

namespace selwonk::vulkan {
// Container that associates each value with an ID, stable for as long as it is
// present in the container
// Used for descriptor arrays
template <typename Impl, typename Key, typename Value,
          typename KeyCmp = std::less<Key>>
class ResourceMap {
public:
  using Handle = Handle;

  Handle get(const Key& key) {
    auto it = mLookup.find(key);
    if (it != mLookup.end())
      return it->second;

    auto newId = nextHandle();
    if (mData.size() <= newId.value())
      mData.resize(newId.value() + 1);
    mData[newId.value()] = static_cast<Impl*>(this)->create(key, newId);
    return newId;
  }

  size_t size() { return mData.size() - mFreelist.size(); }

protected:
  // Insert a value directly. Use in cases where a key would not make sense to
  // identify the value. Prefer `get` whenever possible as it deduplicates
  // values.
  Handle insert(Value value) {
    auto newId = nextHandle();
    if (mData.size() <= newId.value())
      mData.resize(newId.value() + 1);
    mData[newId.value()] = std::move(value);
    return newId;
  }

  // Get the next handle that is due for allocation
  Handle nextHandle() {
    // Reuse freed IDs if possible
    if (!mFreelist.empty()) {
      auto v = mFreelist.back();
      mFreelist.pop_back();
      return Handle(v);
    }
    return Handle(mData.size());
  }

  std::vector<Value> mData;
  std::map<Key, Handle, KeyCmp> mLookup;
  std::vector<Handle::Backing> mFreelist;
};
} // namespace selwonk::vulkan
