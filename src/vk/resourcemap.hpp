#pragma once

#include <cstdint>
#include <limits>
#include <map>
#include <unordered_set>
#include <vector>

namespace selwonk::vulkan {
// Container that associates each value with an ID, stable for as long as it is
// present in the container
// Used for descriptor arrays
template <typename Impl, typename Key, typename Value,
          typename KeyCmp = std::less<Key>>
class ResourceMap {
public:
  using HandleBacking = uint32_t;
  class Handle {
  public:
    const static constexpr HandleBacking InvalidValue =
        std::numeric_limits<HandleBacking>::max();

    explicit Handle(HandleBacking v) : mValue(v) {}
    Handle() : mValue(InvalidValue) {}
    constexpr HandleBacking value() const {
      assert(valid());
      return mValue;
    }
    constexpr bool valid() const { return mValue != InvalidValue; }

  private:
    HandleBacking mValue;
  };

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

protected:
  // Get the next handle that is due for allocation
  Handle nextHandle() {
    // Reuse freed IDs if possible
    if (!mFreelist.empty()) {
      auto v = *mFreelist.begin();
      mFreelist.erase(v);
      return Handle(v);
    }
    return Handle(mData.size());
  }

  std::vector<Value> mData;
  std::map<Key, Handle, KeyCmp> mLookup;
  std::unordered_set<HandleBacking> mFreelist;
};
} // namespace selwonk::vulkan
