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

const void Camera::onEcsAdd() const
{
  auto& handles = assets::INativeHandleProvider::get();
  handles.incRef(mImages.draw);
  handles.incRef(mImages.depth);
}

const void Camera::onEcsRemove() const
{
  auto& handles = assets::INativeHandleProvider::get();
  handles.decRef(mImages.draw);
  handles.decRef(mImages.depth);
}

glm::mat4 Camera::getMatrix() const {
  glm::mat4 out;
  float aspect = (float)mSize.x / mSize.y;
  switch (mType) {
  case ProjectionType::Perspective:
    out = glm::perspective(mFov, aspect,
      // Inverse near and far to improve quality, and
      // avoid wasting precision near the camera
      /*zNear=*/mFar, /*zFar=*/mNear);
  }

  // Invert the Y axis to match Vulkan's coordinate system
  // This can't easily be done on the mesh side without recalculating normals
  out[1][1] *= -1;
  return out;
}
} // namespace selwonk::ecs
