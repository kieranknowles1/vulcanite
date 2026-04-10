#pragma once

#include "entity.hpp"
#include "system.hpp"

#include "../core/keyboard.hpp"
#include "vncore/vfs.hpp"

#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/vector_float3.hpp>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

// TODO: Single header for JSON/GLM interop
namespace glm {
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(vec3, x, y, z)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(quat, x, y, z, w)
} // namespace glm

namespace nlohmann {
template <> struct adl_serializer<selwonk::core::Duration> {
  static void to_json(json& j, const selwonk::core::Duration& value) {
    j = value.count();
  }

  static void from_json(const json& j, selwonk::core::Duration& value) {
    value = selwonk::core::Duration(j.get<long>());
  }
};
} // namespace nlohmann

namespace selwonk::ecs {
class CameraPathSystem : public System {
public:
  // TODO: Configurable path, lock behind cvar
  CameraPathSystem(ecs::EntityRef camera, const core::Keyboard& keyboard,
                   core::Vfs::SubdirPath& file);

  ~CameraPathSystem();

  // TODO: Move to ECS serailisation
  struct Node {
    glm::vec3 mPosition;
    glm::quat mRotation;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Node, mPosition, mRotation)
  };

  void update(ecs::Registry& ecs, core::Duration dt) override;
  std::string_view name() const noexcept override { return "CameraPath"; }

private:
  ecs::EntityRef mCamera;
  const core::Keyboard& mKeyboard;
  ecs::EntityRef mCurrentNode;
  ecs::EntityRef mStartingNode;
  core::Duration mCurrentNodeTime = core::Duration::zero();
};
} // namespace selwonk::ecs
