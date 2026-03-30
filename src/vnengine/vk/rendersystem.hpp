#pragma once

#include "../ecs/camera.hpp"
#include "../ecs/renderable.hpp"
#include "../ecs/system.hpp"
#include "../ecs/transform.hpp"
#include "vncore/bumpallocator.hpp"
#include <vulkan/vulkan.hpp>

namespace selwonk::vulkan {
class VulkanEngine;

class RenderSystem : public ecs::System {
public:
  RenderSystem(VulkanEngine& engine);

  void update(ecs::Registry& registry, Duration dt) override;
  std::optional<std::string_view> blocksBarriers() const noexcept override {
    return "Rendering must see the final world state; no barriers or writes "
           "are allowed after its execution";
  }
  std::string_view name() const noexcept override { return "Render"; }

private:
  void drawScene(const ecs::Transform& cameraTransform,
                 const ecs::Camera& camera);
  void drawBackground(vk::CommandBuffer cmd);
  void draw(const ecs::Transform& cameraTransform, const ecs::Camera& camera);

  void drawSurface(const glm::mat4& modelMatrix, const Mesh& mesh,
                   const Mesh::Surface& surface, core::BumpAllocator& allocator,
                   unsigned int index);

  void beginRenderPipeline(vk::CommandBuffer cmd, vk::Pipeline pipeline);

  struct TransparentDrawData {
    float cameraDistanceSquared;
    glm::mat4 modelMatrix;
    const Mesh* mesh;
    const Mesh::Surface* surface;

    const constexpr bool operator<(const TransparentDrawData& other) const {
      return cameraDistanceSquared < other.cameraDistanceSquared;
    }
  };
  // Keep transparent data allocated between frames to reduce allocation load
  std::vector<TransparentDrawData> mTransparent;

  VulkanEngine& mEngine;
};
} // namespace selwonk::vulkan
