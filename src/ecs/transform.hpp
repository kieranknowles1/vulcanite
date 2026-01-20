#pragma once

#include "component.hpp"
#include "entity.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace selwonk::ecs {
class Registry;

struct Transform {
  struct SetTransform;

  const static constexpr ComponentType Type = ComponentType::Transform;
  const static constexpr char* Name = "Transform";
  using Store = ComponentArray<Transform>;

  glm::mat4 modelMatrix() const {
    return glm::translate(glm::mat4(1.0f), mTranslation) * rotationMatrix() *
           glm::scale(glm::mat4(1.0f), mScale);
  }

  Transform apply(const Transform& other) const {
    Transform result;
    result.mScale = mScale * other.mScale;
    result.mRotation = mRotation * other.mRotation;
    result.mTranslation = mTranslation + (mRotation * other.mTranslation);
    return result;
  }

  constexpr bool operator==(const Transform& other) const noexcept {
    return other.mTranslation == mTranslation && other.mRotation == mRotation &&
           other.mScale == mScale;
  }

  glm::mat4 rotationMatrix() const { return glm::mat4_cast(mRotation); }

  glm::vec3 mTranslation = glm::vec3(0.0f);
  glm::quat mRotation = glm::identity<glm::quat>();
  glm::vec3 mScale = glm::vec3(1.0f);
};

struct Transform::SetTransform {
  EntityRef mTarget;
  Transform mNewData;

  void apply(Registry& ecs);
};

} // namespace selwonk::ecs
