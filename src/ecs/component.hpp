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

// Total number of component types
const static constexpr std::underlying_type_t<ComponentType> ComponentCount =
    static_cast<std::underlying_type_t<ComponentType>>(ComponentType::Max);

// Mask that can hold present/absent for each component type
using ComponentMask = std::bitset<ComponentCount>;

// A mask representing a non-existent entity
const static constexpr ComponentMask NullMask = ComponentMask(0);

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
