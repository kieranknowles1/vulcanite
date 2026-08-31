#pragma once

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/vector_uint2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

#include "component.hpp"
#include "entity.hpp"
#include "vnassets/image.hpp"

namespace selwonk::ecs {
class Registry;

struct Camera {
  struct SetData;

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
  glm::uvec2 mSize;
  assets::ImageBase::Handle mDraw;
  assets::ImageBase::Handle mDepth;

  glm::mat4 getMatrix() const {
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
};

struct Camera::SetData {
  EntityRef mTarget;
  Camera mData;

  void apply(Registry& ecs);
};

} // namespace selwonk::ecs
