#pragma once

#include <cmath>
#include <memory>
#include <tuple>
#include <variant>

#include <spdlog/spdlog.h>

#include <vncore/times.hpp>

#include "applycommandssystem.hpp"
#include "component.hpp"
#include "entity.hpp"

#include "camera.hpp"
#include "link.hpp"
#include "named.hpp"
#include "renderable.hpp"
#include "system.hpp"
#include "transform.hpp"

namespace selwonk::ecs {
template <typename T>
concept HasEcsAdd = requires(T& t) { t.onEcsAdd(); };
template <typename T>
concept HasEcsRemove = requires(T& t) { t.onEcsRemove(); };

class Registry {
public:
  using ComponentArrayTuple =
      std::tuple<Transform::Store, Named::Store, Renderable::Store,
                 Camera::Store, Link::Store>;

  using CommandVariant =
      std::variant<Camera::SetTarget, Transform::SetTransform>;

  ComponentMask getComponentMask(EntityRef entity) const;

  ~Registry();

  // TODO: Remove non-const versions of everything we can
  // TODO: Check that refs and ptrs are const
  // TODO: Support ptrs for optionals
  template <typename... Components, typename F, bool includeDisabled = false>
    requires std::invocable<F&, EntityRef, Components...> &&
             ((std::is_reference_v<Components> ||
               std::is_pointer_v<Components>) &&
              ...)
  void forEach(F&& callback) {
    auto mask = searchMask<Components...>(includeDisabled);
    for (EntityRef::Id entity = 0; entity < mNextEntityId; entity++) {
      if (mComponentMasks[entity].matches(mask)) {
        callback(entity, (fetchComponent<Components>(entity))...);
      }
    }
  }

  template <typename... Components>
  static consteval ComponentMask searchMask(bool includeDisabled) {
    ComponentMask mask{};
    // Filter out disabled and dead entities
    mask.setFlag(EntityFlag::Alive, true);
    mask.setFlag(EntityFlag::Enabled, !includeDisabled);
    ((
        [&] {
          using U = std::remove_reference_t<Components>;
          if constexpr (std::is_reference_v<Components>) {
            mask.setComponentPresent(U::Type, true);
          }
        }(),
        ...));
    return mask;
  }

  template <typename T> bool hasComponent(EntityRef entity) const {
    return getComponentMask(entity).hasComponent(T::Type);
  }
  bool alive(EntityRef entity) const {
    return getComponentMask(entity).hasFlag(EntityFlag::Alive);
  }
  constexpr void setEnabled(EntityRef entity, bool enabled) {
    mComponentMasks[entity.id()].setFlag(EntityFlag::Enabled, enabled);
  }

  EntityRef createEntity();
  void deleteEntity(EntityRef entity);

  template <typename T>
  void addComponent(EntityRef entity, const T& component) {
    checkAlive(entity);
    // spdlog::("Add {} to {}", T::Name, entity.id());

    if constexpr (HasEcsAdd<T>) {
      component.onEcsAdd();
    }

    getComponentArray<T>().add(entity, component);
    mComponentMasks[entity.id()].setComponentPresent(T::Type, true);
  }

  template <typename T> void removeComponent(EntityRef entity) {
    checkAlive(entity);

    auto& component = getComponentArray<T>().get(entity);
    if constexpr (HasEcsRemove<T>) {
      component.onEcsRemove();
    }
    mComponentMasks[entity.id()].setComponentPresent(T::Type, false);
  }

  template <typename T> const T& getComponent(EntityRef entity) {
    checkAlive(entity);
    assert(hasComponent<T>(entity));
    return getComponentArray<T>().get(entity);
  }
  template <typename T> const T* tryGetComponent(EntityRef entity) {
    checkAlive(entity);
    if (hasComponent<T>(entity)) {
      return &getComponentArray<T>().get(entity);
    }
    return nullptr;
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
    SPDLOG_INFO("Add system {}", system->name());
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

  void update(core::Duration dt);

  void queueCommand(const CommandVariant&& cmd) {
    assert(!debug_commandsBlocked &&
           "Commands may only be queued during an update and before any system "
           "that blocks barriers");
    mQueuedCommands.emplace_back(cmd);
  }

  void executeImmediate(CommandVariant&& cmd);

private:
  template <typename T>
    requires std::is_pointer_v<T>
  const T fetchComponent(EntityRef entity) {
    return tryGetComponent<std::remove_pointer_t<T>>(entity);
  }
  template <typename T>
    requires std::is_reference_v<T>
  const T fetchComponent(EntityRef entity) {
    return getComponent<std::remove_reference_t<T>>(entity);
  }

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
  System* mCommandBlocker = nullptr;

#ifndef NDEBUG
  bool debug_commandsBlocked = false;
  bool debug_updating = false;
  bool debug_barrierActive = false;
#endif
};
} // namespace selwonk::ecs
