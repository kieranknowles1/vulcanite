#pragma once

#include "system.hpp"

namespace selwonk::ecs {
class ApplyCommandsSystem : public System {
  void update(Registry& ecs, Duration dt) override;
  std::string_view name() const noexcept override { return "ApplyCommands"; }
};
} // namespace selwonk::ecs
