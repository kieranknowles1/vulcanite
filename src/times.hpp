#pragma once

#include <chrono>

namespace selwonk {
using Duration = std::chrono::nanoseconds;

constexpr uint64_t chronoToVulkan(const Duration& duration) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

constexpr float seconds(Duration duration) {
  return std::chrono::duration_cast<std::chrono::duration<float>>(duration)
      .count();
}

const static constexpr uint64_t RenderTimeout =
    chronoToVulkan(std::chrono::seconds(1));

} // namespace selwonk
