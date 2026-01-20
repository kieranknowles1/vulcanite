#pragma once

#include "system.hpp"

namespace selwonk::ecs {
class ApplyCommandsSystem : public System {
  void update(Registry& ecs, Duration dt);
};
} // namespace selwonk::ecs
