#include "applycommandssystem.hpp"
#include "registry.hpp"

namespace selwonk::ecs {
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

void Registry::update(Duration dt) {
  assert(mCommandBarrierCount > 0 &&
         "The ECS must have at least one command barrier");
#ifndef NDEBUG
  debug_commandsBlocked = false;
#endif

  for (auto& system : mSystems) {
#ifndef NDEBUG
    debug_commandsBlocked |= system->blocksBarriers() != std::nullopt;
    debug_barrierActive =
        dynamic_cast<ApplyCommandsSystem*>(system.get()) != nullptr;
#endif

    system->update(*this, dt);
  }

  assert(mQueuedCommands.empty() &&
         "The last command barrier must appear after the last system that "
         "writes commands");
#ifndef NDEBUG
  // Allow writes outside of updates, for exceptional cases where a system would
  // be overkill such as updating the camera's target after a resize
  debug_commandsBlocked = false;
#endif
}

} // namespace selwonk::ecs
