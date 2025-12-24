#pragma once

#include <chrono>

namespace selwonk {
using Duration = std::chrono::duration<float>;

// FIXME: Should be using std::chrono::duration
constexpr uint64_t millisToNanoSeconds(uint64_t millis) {
  return millis * 1000000;
}
} // namespace selwonk
