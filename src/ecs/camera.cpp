#include "camera.hpp"

#include "registry.hpp"

namespace selwonk::ecs {
void Camera::SetTarget::apply(Registry& ecs) {
  auto component = ecs.getComponentMutable<Camera>(mTarget);
  component.mDrawTarget = mDraw;
  component.mDepthTarget = mDepth;
}
} // namespace selwonk::ecs
