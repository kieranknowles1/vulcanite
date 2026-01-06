#pragma once

#include <cmath>
#include <tuple>

#include "component.hpp"
#include "entity.hpp"

#include "camera.hpp"
#include "named.hpp"
#include "renderable.hpp"
#include "transform.hpp"

namespace selwonk::ecs {
class Registry {
public:
  using ComponentArrayTuple = std::tuple<Transform::Store, Named::Store,
                                         Renderable::Store, Camera::Store>;

  ComponentMask getComponentMask(EntityRef entity);

  template <typename... Components, typename F, bool includeDisabled = false>
  void forEach(F&& callback) {
    auto mask = searchMask<Components...>(includeDisabled);
    for (EntityRef::Id entity = 0; entity < mNextEntityId; entity++) {
      if (mComponentMasks[entity].matches(mask)) {
        callback(entity, (getComponentArray<Components>().get(entity))...);
      }
    }
  }

  template <typename... Components>
  static consteval ComponentMask searchMask(bool includeDisabled) {
    ComponentMask mask{};
    mask.setFlag(EntityFlag::Alive, true);
    // Filter out disabled components
    mask.setFlag(EntityFlag::Enabled, !includeDisabled);
    ((mask.setComponentPresent(Components::Type, true), ...));
    return mask;
  }

  template <typename T> bool hasComponent(EntityRef entity) {
    return getComponentMask(entity).hasComponent(T::Type);
  }
  bool alive(EntityRef entity) {
    return getComponentMask(entity).hasFlag(EntityFlag::Alive);
  }
  constexpr void setEnabled(EntityRef entity, bool enabled) {
    mComponentMasks[entity.id()].setFlag(EntityFlag::Enabled, enabled);
  }

  EntityRef createEntity();

  template <typename T>
  void addComponent(EntityRef entity, const T& component) {
    checkAlive(entity);
    // fmt::println("Add {} to {}", T::Name, entity.id());

    getComponentArray<T>().add(entity, component);
    mComponentMasks[entity.id()].setComponentPresent(T::Type, true);
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

  template <typename T> T::Store& getComponentArray() {
    return std::get<typename T::Store>(mComponentArrays);
  }

  ComponentArrayTuple mComponentArrays;

  EntityRef::Id mNextEntityId = 0;
  std::vector<ComponentMask> mComponentMasks;
};
} // namespace selwonk::ecs
