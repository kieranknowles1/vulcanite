#include "applycommandssystem.hpp"

#include "registry.hpp"

namespace selwonk::ecs {

void ApplyCommandsSystem::update(ecs::Registry& ecs, Duration dt) {
  auto& queue = ecs.getQueuedCommands();
  for (auto& cmd : queue) {
    std::visit([&](auto& val) { val.apply(ecs); }, cmd);
  }
  queue.clear();
}

} // namespace selwonk::ecs
