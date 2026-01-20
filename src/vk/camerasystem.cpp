#include "camerasystem.hpp"

#include "../ecs/registry.hpp"
#include <fmt/base.h>

namespace selwonk::vulkan {
void CameraSystem::update(ecs::Registry& ecs, Duration dt) {
  // TODO: Sensitivity should be a cvar
  float mouseSensitivity = 0.03f;
  auto& playerPos = ecs.getComponent<ecs::Transform>(mCamera);

  mSpeed += mKeyboard.getAnalog(core::Keyboard::AnalogControl::SpeedChange) *
            seconds(dt) * 5.0f;
  if (mKeyboard.getDigital(core::Keyboard::DigitalControl::ToggleMouse)) {
    mWindow.setMouseVisible(!mWindow.mouseVisible());
  }

  glm::vec3 movement = {
      -mKeyboard.getAnalog(core::Keyboard::AnalogControl::MoveLeftRight),
      0,
      -mKeyboard.getAnalog(core::Keyboard::AnalogControl::MoveForwardBackward),
  };

  if (!mWindow.mouseVisible()) {
    mPitch -= mKeyboard.getAnalog(core::Keyboard::AnalogControl::LookUpDown) *
              mouseSensitivity;
    mPitch = glm::clamp(mPitch, -glm::half_pi<float>(), glm::half_pi<float>());
    mYaw -= mKeyboard.getAnalog(core::Keyboard::AnalogControl::LookLeftRight) *
            mouseSensitivity;
    mYaw = glm::mod(mYaw, glm::two_pi<float>());
  }

  ecs::Transform::SetTransform cmd{
      .mTarget = mCamera,
      .mNewData = playerPos,
  };

  cmd.mNewData.mRotation = glm::quat(glm::vec3(mPitch, mYaw, 0.0f));
  cmd.mNewData.mTranslation += playerPos.rotationMatrix() *
                               glm::vec4(movement, 0.0f) * seconds(dt) * mSpeed;
  if (cmd.mNewData != playerPos) {
    ecs.queueCommand(cmd);
  }
}

} // namespace selwonk::vulkan
