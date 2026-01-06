#pragma once

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

#include "../vk/image.hpp"
#include "component.hpp"

namespace selwonk::ecs {
struct Camera {
  const static constexpr ComponentType Type = ComponentType::Camera;
  const static constexpr char* Name = "Camera";
  using Store = SparseComponentArray<Camera>;

  enum class ProjectionType : uint8_t {
    Perspective,
  };

  ProjectionType mType;
  float mNear;
  float mFar;
  // In radians
  float mFov;
  std::shared_ptr<vulkan::Image> mDrawTarget;
  std::shared_ptr<vulkan::Image> mDepthTarget;

  constexpr glm::mat4 getMatrix() const {
    assert(mDrawTarget->getExtent() == mDepthTarget->getExtent() &&
           "Draw and depth targets should be the same size");

    glm::mat4 out;
    float aspect = (float)mDrawTarget->getExtent().width /
                   (float)mDrawTarget->getExtent().height;
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
};
} // namespace selwonk::ecs
