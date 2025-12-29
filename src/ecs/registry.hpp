#pragma once

#include <cmath>
#include <tuple>

#include "component.hpp"
#include "entity.hpp"

#include "transform.hpp"

namespace selwonk::ecs {
class Registry {
public:
  using ComponentArrayTuple = std::tuple<ComponentArray<Transform>>;

  EntityId createEntity() {
    EntityId id = mNextEntityId;
    mComponents.resize(std::max(id + 1, (EntityId)mComponents.size()));
    mNextEntityId++;
    return id;
  }

  void assertAlive(EntityId entity) { assert(entity < mNextEntityId); }

  template <typename T> T &addComponent(EntityId entity) {
    assertAlive(entity);
    auto &component = getComponentArray<T>().getOrAdd(entity);
    mComponents[entity].set(static_cast<size_t>(T::Type));
    return component;
  }

  template <typename T> T &getComponent(EntityId entity) {
    assertAlive(entity);
    assert(hasComponent(entity, T::Type));
    return getComponentArray<T>().get(entity);
  }

  bool hasComponent(EntityId entity, ComponentType type) {
    if (entity >= mNextEntityId) {
      return false;
    }
    return mComponents[entity].test(static_cast<size_t>(type));
  }

private:
  template <typename T> ComponentArray<T> &getComponentArray() {
    return std::get<ComponentArray<T>>(mComponentArrays);
  }

  ComponentArrayTuple mComponentArrays;

  EntityId mNextEntityId = 0;
  std::vector<ComponentMask> mComponents;
};
} // namespace selwonk::ecs
