#include "registry.hpp"

namespace selwonk::ecs {
ComponentMask Registry::getComponentMask(EntityId entity) {
  if (entity >= mComponentMasks.size())
    return NullMask;
  return mComponentMasks[entity];
}

EntityId Registry::createEntity() {
  EntityId id = mNextEntityId;
  mComponentMasks.resize(std::max(id + 1, (EntityId)mComponentMasks.size()));
  mNextEntityId++;

  mComponentMasks[id].set(static_cast<size_t>(ComponentType::Alive));

  return id;
}

} // namespace selwonk::ecs
