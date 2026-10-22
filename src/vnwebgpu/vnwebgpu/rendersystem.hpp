#pragma once

#include <vnecs/system.hpp>

namespace selwonk::webgpu {
class RenderSystem : public ecs::System {

public:
  RenderSystem() = default;

  // TODO: Implement
  void update(ecs::Registry& registry, core::Duration dt) override {};
  std::optional<std::string_view> blocksBarriers() const noexcept override {
    return "Rendering must see the final world state; no barriers or writes "
           "are allowed after its execution";
  }

  std::string_view name() const noexcept override { return "Render"; }
};
} // namespace selwonk::webgpu
