#pragma once

#include <chrono>
#include <string_view>

#include "ringbuffer.hpp"

namespace selwonk::core {
// Basic profiler. Assumes the same sections are present every frame
class Profiler {
public:
  using Clock = std::chrono::high_resolution_clock;

  void beginFrame();
  void endFrame();
  // Print metrics over ImGui
  void printTimes();

  // Begin recording a new section
  void startSection(std::string_view name);

private:
  const static constexpr int Samples = 128;
  struct Metric {
    std::string mName;
    RingBuffer<Clock::duration, Samples> mSamples;
  };

  // Get time elapsed since last call to this function, or the beginning of the
  // current frame
  Clock::duration getElapsed();

  std::vector<Metric> mMetrics;
  size_t mNextSectionIndex = 0;
  Clock::time_point mLastSectionEnd;
};
} // namespace selwonk::core
