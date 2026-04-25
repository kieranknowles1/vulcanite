#include "registry.hpp"

#include "applycommandssystem.hpp"
#include "component.hpp"
#include "entity.hpp"
#include <vncore/profiler.hpp>

namespace selwonk::ecs {
Registry::~Registry() {
  for (EntityRef::Id id = 0; id < mComponentMasks.size(); id++) {
    EntityRef entity(id);
    if (alive(entity)) {
      deleteEntity(entity);
    }
  }
}

ComponentMask Registry::getComponentMask(EntityRef entity) {
  if (entity.id() >= mComponentMasks.size())
    return ComponentMask::null();
  return mComponentMasks[entity.id()];
}

EntityRef Registry::createEntity() {
  EntityRef::Id id = mNextEntityId;
  mComponentMasks.resize(
      std::max(id + 1, (EntityRef::Id)mComponentMasks.size()));
  mNextEntityId++;

  mComponentMasks[id].setFlag(EntityFlag::Alive, true);
  mComponentMasks[id].setFlag(EntityFlag::Enabled, true);

  return EntityRef(id);
}

void Registry::deleteEntity(EntityRef entity) {
  checkAlive(entity);
  mComponentMasks[entity.id()].setFlag(EntityFlag::Alive, false);

  auto deleteImpl = [&](auto& array) {
    using Type = std::decay_t<decltype(array)>::ValueType;
    if constexpr (HasEcsRemove<Type>) {
      if (hasComponent<Type>(entity)) {
        array.get(entity).onEcsRemove();
      }
    }
  };

  std::apply([&](auto&... arrays) { (deleteImpl(arrays), ...); },
             mComponentArrays);
}

void Registry::update(core::Duration dt) {
  assert(mCommandBarrierCount > 0 &&
         "The ECS must have at least one command barrier");
#ifndef NDEBUG
  debug_commandsBlocked = false;
  debug_barrierActive = false;
  debug_updating = true;
#endif

  auto& profiler = core::Profiler::get();
  for (auto& system : mSystems) {
    profiler.pushSection(system->name());

#ifndef NDEBUG
    debug_commandsBlocked |= system->blocksBarriers() != std::nullopt;
    debug_barrierActive =
        dynamic_cast<ApplyCommandsSystem*>(system.get()) != nullptr;
#endif

    system->update(*this, dt);
    profiler.popSection();
  }

  assert(mQueuedCommands.empty() &&
         "The last command barrier must appear after the last system that "
         "writes commands");
#ifndef NDEBUG
  // Allow writes outside of updates, for exceptional cases where a system would
  // be overkill such as updating the camera's target after a resize
  debug_updating = false;
#endif
}

void Registry::executeImmediate(CommandVariant&& cmd) {
#ifndef NDEBUG
  assert(!debug_updating && "executeImmediate is not allowed during update");
  debug_barrierActive = true;
#endif
  std::visit([&](auto& val) { val.apply(*this); }, cmd);
#ifndef NDEBUG
  debug_barrierActive = false;
#endif
}

} // namespace selwonk::ecs
