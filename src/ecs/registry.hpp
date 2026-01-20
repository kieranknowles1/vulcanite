#pragma once

#include <cmath>
#include <memory>
#include <tuple>
#include <variant>

#include "../times.hpp"
#include "applycommandssystem.hpp"
#include "component.hpp"
#include "entity.hpp"

#include "camera.hpp"
#include "named.hpp"
#include "renderable.hpp"
#include "system.hpp"
#include "transform.hpp"

namespace selwonk::ecs {
class Registry {
public:
  using ComponentArrayTuple = std::tuple<Transform::Store, Named::Store,
                                         Renderable::Store, Camera::Store>;

  using CommandVariant =
      std::variant<Camera::SetTarget, Transform::SetTransform>;

  ComponentMask getComponentMask(EntityRef entity);

  // TODO: Remove non-const version
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

  template <typename T> const T& getComponent(EntityRef entity) {
    checkAlive(entity);
    assert(hasComponent<T>(entity));
    return getComponentArray<T>().get(entity);
  }

  // Get a mutable component reference, must only be called when applying a
  // barrier
  template <typename T> T& getComponentMutable(EntityRef entity) {
    assert(debug_barrierActive &&
           "getComponentMutable is only allowed during barrier application");
    checkAlive(entity);
    assert(hasComponent<T>(entity));
    return getComponentArray<T>().get(entity);
  }

  // Get queued commands, must only be called when applying a
  // barrier
  std::vector<CommandVariant>& getQueuedCommands() {
    assert(debug_barrierActive &&
           "getQueuedCommands is only allowed during barrier application");
    return mQueuedCommands;
  }

  const ComponentArrayTuple& getComponentArrays() const {
    return mComponentArrays;
  }

  template <typename T> T* addSystem(std::unique_ptr<T> system) {
    auto ptr = system.get();
    mSystems.emplace_back(std::move(system));

    auto block = ptr->blocksBarriers();
    if (block != std::nullopt && mCommandBlocker == nullptr) {
      mCommandBlocker = ptr;
    }

    return ptr;
  }

  // Add a barrier that forces all previous systems to finish before continuing,
  // then apply commands from them
  // As we don't currently multithread, this just adds the system for now
  // There must be at least one barrier, otherwise changes will never be written
  // There may be multiple barriers if systems are interdependent, but overuse
  // should be seen as a code smell
  ApplyCommandsSystem* addCommandBarrier() {
    mCommandBarrierCount += 1;
    assert(mCommandBlocker == nullptr &&
           "A system has forbidden the use of further barriers");
    return addSystem(std::make_unique<ApplyCommandsSystem>());
  }

  void update(Duration dt);

  void queueCommand(const CommandVariant&& cmd) {
    assert(!debug_commandsBlocked);
    mQueuedCommands.emplace_back(cmd);
  }

private:
  void checkAlive(EntityRef entity) { assert(alive(entity)); }

  template <typename T> T::Store& getComponentArray() {
    return std::get<typename T::Store>(mComponentArrays);
  }

  ComponentArrayTuple mComponentArrays;
  std::vector<CommandVariant> mQueuedCommands;

  EntityRef::Id mNextEntityId = 0;
  std::vector<ComponentMask> mComponentMasks;
  std::vector<std::unique_ptr<System>> mSystems;

  int mCommandBarrierCount = 0;
  System* mCommandBlocker;

#ifndef NDEBUG
  bool debug_commandsBlocked = false;
  bool debug_barrierActive = false;
#endif
};
} // namespace selwonk::ecs
