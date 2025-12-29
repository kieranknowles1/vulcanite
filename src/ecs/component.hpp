#pragma once

#include <bitset>
#include <cstdint>
#include <vector>

#include "entity.hpp"

namespace selwonk::ecs {
enum class ComponentType : uint8_t {
  Alive,
  Transform,
  Renderable,
  Max,
};
const static constexpr uint8_t ComponentCount =
    static_cast<uint8_t>(ComponentType::Max);

using ComponentMask = std::bitset<ComponentCount>;
const static constexpr ComponentMask NullMask =
    ComponentMask(static_cast<size_t>(ComponentType::Alive));

template <typename T> class ComponentArray {
public:
  void add(EntityId entity, const T &value) {
    if (mComponents.size() <= entity) {
      mComponents.resize(entity + 1);
    }
    mComponents[entity] = value;
  }
  T &get(EntityId entity) {
    assert(entity < mComponents.size());
    return mComponents[entity];
  }

private:
  std::vector<T> mComponents;
};

} // namespace selwonk::ecs
