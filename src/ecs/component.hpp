#pragma once

#include <bitset>
#include <cstdint>
#include <vector>

#include "entity.hpp"

namespace selwonk::ecs {
enum class ComponentType : uint8_t {
  Transform,
  Max,
};
const static constexpr uint8_t ComponentCount =
    static_cast<uint8_t>(ComponentType::Max);
using ComponentMask = std::bitset<ComponentCount>;

template <typename T> class ComponentArray {
public:
  T &getOrAdd(EntityId entity) {
    if (mComponents.size() <= entity) {
      mComponents.resize(entity + 1);
    }
    return mComponents[entity];
  }
  T &get(EntityId entity) {
    assert(entity < mComponents.size());
    return mComponents[entity];
  }

private:
  std::vector<T> mComponents;
};

} // namespace selwonk::ecs
