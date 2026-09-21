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

  struct Images {
    assets::ImageBase::Handle draw;
    assets::ImageBase::Handle depth;
  };

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
  Images mImages;

  const void onEcsAdd() const;
  const void onEcsRemove() const;

  glm::mat4 getMatrix() const;
};

struct Camera::SetData {
  EntityRef mTarget;
  Camera mData;

  void apply(Registry& ecs);
};

} // namespace selwonk::ecs
