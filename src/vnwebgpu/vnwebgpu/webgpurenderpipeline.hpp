#pragma once

#include <vnassets/irenderpipeline.hpp>

#include "rendersystem.hpp"
#include "webgpunativehandleprovider.hpp"

namespace selwonk::webgpu {

class WebGpuRenderPipeline : public assets::IRenderPipeline {
public:
  // Create a set of images for use as a camera's target
  // TODO: Implement
  ecs::Camera::Images createDrawImage(glm::uvec2 size) override { return{}; };

  // TODO: Implement
  std::unique_ptr<ecs::System> createRenderSystem() override { return std::make_unique<RenderSystem>(); };

  // TODO: Implement
  void present(const ecs::Camera& mainCamera) override {};
  // TODO: Implement
  void waitIdle() const override {};

  // Hook for frame begin. Runs after UI (wiki:[[VNEngine#Frame Process]])
  // TODO: Implement
  void beginFrame() override {};

private:
  WebGpuNativeHandleProvider mNativeHandles;
};

}
