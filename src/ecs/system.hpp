#pragma once

#include "../times.hpp"

namespace selwonk::ecs {
class Registry;

class System {
public:
  virtual void update(ecs::Registry& registry, Duration dt) = 0;
  virtual ~System() = default;
};
} // namespace selwonk::ecs
