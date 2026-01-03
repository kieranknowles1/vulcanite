#pragma once

#include <cmath>
#include <tuple>

#include "component.hpp"
#include "entity.hpp"

#include "matrixtransform.hpp"
#include "named.hpp"
#include "renderable.hpp"
#include "transform.hpp"

namespace selwonk::ecs {
class Registry {
public:
  using ComponentArrayTuple =
      std::tuple<ComponentArray<Transform>, ComponentArray<MatrixTransform>,
                 ComponentArray<Named>, ComponentArray<Renderable>>;

  ComponentMask getComponentMask(EntityRef entity);

  template <typename... Components, typename F> void forEach(F&& callback) {
    auto mask = componentMask<Components...>();
    for (EntityRef::Id entity = 0; entity < mNextEntityId; ++entity) {
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

  template <typename T> bool hasComponent(EntityRef entity) {
    return getComponentMask(entity).test(static_cast<size_t>(T::Type));
  }
  bool alive(EntityRef entity) {
    return getComponentMask(entity).test(
        static_cast<size_t>(ComponentType::Alive));
  }

  EntityRef createEntity();

  template <typename T> void addComponent(EntityRef entity, T component) {
    checkAlive(entity);
    getComponentArray<T>().add(entity, component);
    mComponentMasks[entity.id()].set(static_cast<size_t>(T::Type));
  }

  template <typename T> T& getComponent(EntityRef entity) {
    checkAlive(entity);
    assert(hasComponent<T>(entity));
    return getComponentArray<T>().get(entity);
  }

  const ComponentArrayTuple& getComponentArrays() const {
    return mComponentArrays;
  }

private:
  void checkAlive(EntityRef entity) { assert(alive(entity)); }

  template <typename T> ComponentArray<T>& getComponentArray() {
    return std::get<ComponentArray<T>>(mComponentArrays);
  }

  ComponentArrayTuple mComponentArrays;

  EntityRef::Id mNextEntityId = 0;
  std::vector<ComponentMask> mComponentMasks;
};
} // namespace selwonk::ecs
