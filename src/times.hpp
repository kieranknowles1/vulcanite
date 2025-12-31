#pragma once

#include <chrono>

namespace selwonk {
using Duration = std::chrono::duration<uint64_t>;

constexpr uint64_t chronoToVulkan(const Duration& duration) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}
} // namespace selwonk
