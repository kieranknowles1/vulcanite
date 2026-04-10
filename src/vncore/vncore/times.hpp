#pragma once

#include <chrono>

namespace selwonk::core {
using Duration = std::chrono::nanoseconds;

constexpr uint64_t chronoToNano(const Duration& duration) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

constexpr float seconds(Duration duration) {
  return std::chrono::duration_cast<std::chrono::duration<float>>(duration)
      .count();
}

constexpr Duration seconds(float seconds) {
  auto dur = std::chrono::duration<float>(seconds);
  return std::chrono::duration_cast<std::chrono::nanoseconds>(dur);
}

// TODO: Move back to engine
const static constexpr uint64_t RenderTimeout =
    chronoToNano(std::chrono::seconds(1));

} // namespace selwonk::core
