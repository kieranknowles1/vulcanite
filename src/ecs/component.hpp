#pragma once

#include <bitset>
#include <cstdint>
#include <vector>

#include "entity.hpp"

namespace selwonk::ecs {
enum class ComponentType : uint8_t {
  // Entity exists and has not been deleted
  // Deleted entities can not be resurrected
  Alive,
  // Entity is currently active and processed by all systems
  // Systems can opt-in to process disabled entities
  // All entities are enabled by default, and may be disabled/re-enabled at any
  // time
  Enabled,
  Transform,
  Named,
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
  using ValueType = T;
  const char* getTypeName() const { return T::Name; }

  void add(EntityRef entity, const T& value) {
    if (mComponents.size() <= entity.id()) {
      mComponents.resize(entity.id() + 1);
    }
    mComponents[entity.id()] = value;

#ifdef VN_LOGCOMPONENTSTATS
    mSize++;
#endif
  }
  T& get(EntityRef entity) {
    assert(entity.id() < mComponents.size());
    return mComponents[entity.id()];
  }

#ifdef VN_LOGCOMPONENTSTATS
  // Get the number of components of this type
  size_t size() const { return mSize; }

  // Get the number of components allocated
  size_t capacity() const { return mComponents.capacity(); }

#endif

private:
  std::vector<T> mComponents;

#ifdef VN_LOGCOMPONENTSTATS
  size_t mSize = 0;
#endif
};

} // namespace selwonk::ecs
