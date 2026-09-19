#include "camera.hpp"

#include <vnassets/inativehandleprovider.hpp>

#include "registry.hpp"

namespace selwonk::ecs {
void Camera::SetData::apply(Registry& ecs) {
  auto& handle = assets::INativeHandleProvider::get();
  auto& component = ecs.getComponentMutable<Camera>(mTarget);

  // Don't increment, we own the draw/depth images and will be discarded after applying
  //handle.incRef(mData.mImages.draw);
  //handle.incRef(mData.mImages.depth);
  handle.decRef(component.mImages.draw);
  handle.decRef(component.mImages.depth);

  component = mData;
}
} // namespace selwonk::ecs
