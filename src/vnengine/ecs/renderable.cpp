#include "renderable.hpp"

#include "../vk/vulkanengine.hpp"

#include <vnassets/inativehandleprovider.hpp>

namespace selwonk::ecs {
const void Renderable::onEcsAdd() const {
  auto& interop = assets::INativeHandleProvider::get();
  interop.incRef(mMesh);
}

const void Renderable::onEcsRemove() const {
  auto& interop = assets::INativeHandleProvider::get();
  interop.decRef(mMesh);
}
} // namespace selwonk::ecs
