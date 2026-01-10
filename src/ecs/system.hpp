#pragma once

namespace selwonk::ecs {
class Registry;

class System {
public:
  virtual void update(ecs::Registry& registry, float dt) = 0;
  virtual ~System() = default;
};
} // namespace selwonk::ecs
