#pragma once

#include "../core/keyboard.hpp"
#include "../core/window.hpp"
#include "../ecs/entity.hpp"
#include "../ecs/system.hpp"

// TODO: This shouldn't be part of vulkan
namespace selwonk::vulkan {
class CameraSystem : public ecs::System {
public:
  CameraSystem(ecs::EntityRef camera, const core::Keyboard& keyboard,
               core::Window& window)
      : mCamera(camera), mKeyboard(keyboard), mWindow(window) {}

  void update(ecs::Registry& ecs, float dt) override;
  ecs::EntityRef getCamera() const { return mCamera; }

private:
  ecs::EntityRef mCamera;
  const core::Keyboard& mKeyboard;
  // TODO: mouseVisible should be part of keyboard
  core::Window& mWindow;

  float mSpeed = 1.0f;
  float mPitch = 0.0f;
  float mYaw = 0.0f;
};
} // namespace selwonk::vulkan
