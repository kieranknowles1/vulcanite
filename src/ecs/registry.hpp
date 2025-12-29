#pragma once

#include <cmath>
#include <tuple>

#include "component.hpp"
#include "entity.hpp"

#include "renderable.hpp"
#include "transform.hpp"

namespace selwonk::ecs {
class Registry {
public:
  using ComponentArrayTuple =
      std::tuple<ComponentArray<Transform>, ComponentArray<Renderable>>;

  ComponentMask getComponentMask(EntityId entity);

  template <typename... Components, typename F> void forEach(F &&callback) {
    auto mask = componentMask<Components...>();
    for (EntityId entity = 0; entity < mNextEntityId; ++entity) {
      if ((mComponentMasks[entity] & mask) == mask) {
        callback(entity, (getComponentArray<Components>().get(entity))...);
      }
    }
  }

  template <typename... Components> ComponentMask componentMask() const {
    ComponentMask mask{};
    ((mask.set(static_cast<size_t>(Components::Type)), ...));
    return mask;
  }

  template <typename T> bool hasComponent(EntityId entity) {
    return getComponentMask(entity).test(static_cast<size_t>(T::Type));
  }
  bool alive(EntityId entity) {
    return getComponentMask(entity).test(
        static_cast<size_t>(ComponentType::Alive));
  }

  EntityId createEntity();

  template <typename T> void addComponent(EntityId entity, T component) {
    checkAlive(entity);
    getComponentArray<T>().add(entity, component);
    mComponentMasks[entity].set(static_cast<size_t>(T::Type));
  }

  template <typename T> T &getComponent(EntityId entity, T &component) {
    checkAlive(entity);
    assert(hasComponent<T>(entity));
    return getComponentArray<T>().get(entity);
  }

private:
  void checkAlive(EntityId entity) { assert(alive(entity)); }

  template <typename T> ComponentArray<T> &getComponentArray() {
    return std::get<ComponentArray<T>>(mComponentArrays);
  }

  ComponentArrayTuple mComponentArrays;

  EntityId mNextEntityId = 0;
  std::vector<ComponentMask> mComponentMasks;
};
} // namespace selwonk::ecs
