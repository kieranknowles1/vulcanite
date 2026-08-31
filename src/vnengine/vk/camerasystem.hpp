#pragma once

#include <vnecs/entity.hpp>
#include <vnecs/system.hpp>
#include <vnsdl/window.hpp>

// TODO: This shouldn't be part of vulkan
namespace selwonk::vulkan {
class CameraSystem : public ecs::System {
public:
  CameraSystem(ecs::EntityRef camera, const sdl::Keyboard& keyboard,
               sdl::Window& window)
      : mCamera(camera), mKeyboard(keyboard), mWindow(window) {}

  void update(ecs::Registry& ecs, core::Duration dt) override;
  std::string_view name() const noexcept override { return "Camera"; }
  ecs::EntityRef getCamera() const { return mCamera; }

private:
  ecs::EntityRef mCamera;
  const sdl::Keyboard& mKeyboard;
  // TODO: mouseVisible should be part of keyboard
  sdl::Window& mWindow;

  float mSpeed = 10.0f;
  float mPitch = 0.0f;
  float mYaw = 0.0f;
};
} // namespace selwonk::vulkan
