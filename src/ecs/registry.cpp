#include "registry.hpp"

namespace selwonk::ecs {
ComponentMask Registry::getComponentMask(EntityRef entity) {
  if (entity.id() >= mComponentMasks.size())
    return NullMask;
  return mComponentMasks[entity.id()];
}

EntityRef Registry::createEntity() {
  EntityRef::Id id = mNextEntityId;
  mComponentMasks.resize(
      std::max(id + 1, (EntityRef::Id)mComponentMasks.size()));
  mNextEntityId++;

  mComponentMasks[id].set(static_cast<size_t>(ComponentType::Alive));

  return EntityRef(id);
}

} // namespace selwonk::ecs
