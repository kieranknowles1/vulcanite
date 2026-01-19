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
  for (auto& system : mSystems) {
    system->update(*this, dt);
  }
}

} // namespace selwonk::ecs
