#include "camera.hpp"

#include "registry.hpp"

namespace selwonk::ecs {
void Camera::SetData::apply(Registry& ecs) {
  auto& component = ecs.getComponentMutable<Camera>(mTarget);
  component = mData;
  // TODO: Adjust ref counts for draw and depth targets
}
} // namespace selwonk::ecs
