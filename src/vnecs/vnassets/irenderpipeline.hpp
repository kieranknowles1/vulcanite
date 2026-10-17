#pragma once

#include <memory>

#include <glm/ext/vector_uint2.hpp>

#include "../vnecs/camera.hpp"
#include "../vnecs/system.hpp"

namespace selwonk::assets {

// Interface for a render backend. This is implemented in `vnecs.lib` despite being part of assets as it exports using ECS interfaces.
class IRenderPipeline
{
public:
  virtual ~IRenderPipeline() = default;

  // Create a set of images for use as a camera's target
  virtual ecs::Camera::Images createDrawImage(glm::uvec2 size) = 0;

  virtual std::unique_ptr<ecs::System> createRenderSystem() = 0;

  virtual void present(const ecs::Camera& mainCamera) = 0;
  virtual void waitIdle() const = 0;
  size_t getFrameNumber() const { return mFrameNumber; }

  // Hook for frame begin. Runs after UI (wiki:[[VNEngine#Frame Process]])
  virtual void beginFrame() = 0;

protected:
  // Implementations are responsible for incrementing this
  size_t mFrameNumber = 0;
};

}
