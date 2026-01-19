#pragma once

#include "../ecs/camera.hpp"
#include "../ecs/system.hpp"
#include "../ecs/transform.hpp"
#include <vulkan/vulkan.hpp>

namespace selwonk::vulkan {
class VulkanEngine;

class RenderSystem : public ecs::System {
public:
  RenderSystem(VulkanEngine& engine);

  void update(ecs::Registry& registry, Duration dt) override;

private:
  void drawScene(const ecs::Transform& cameraTransform,
                 const ecs::Camera& camera);
  void drawBackground(vk::CommandBuffer cmd);
  void draw(const ecs::Transform& cameraTransform, const ecs::Camera& camera);

  VulkanEngine& mEngine;
};
} // namespace selwonk::vulkan
