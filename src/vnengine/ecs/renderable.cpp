#include "renderable.hpp"

#include "../vk/vulkanengine.hpp"

namespace selwonk::ecs {
const void Renderable::onEcsAdd() const {
  vulkan::VulkanEngine::get().mMeshes.incRef(mMesh);
}

const void Renderable::onEcsRemove() const {
  vulkan::VulkanEngine::get().mMeshes.decRef(mMesh);
}
} // namespace selwonk::ecs
