#include "transform.hpp"

#include "registry.hpp"

namespace selwonk::ecs {

void Transform::SetTransform::apply(Registry& ecs) {
  ecs.getComponentMutable<Transform>(mTarget) = mNewData;
}

} // namespace selwonk::ecs
