#pragma once

#include <vncore/profiler.hpp>

namespace selwonk::core {
  class ProfilerUi {
  public:
    ProfilerUi(const Profiler& profiler)
      : mProfiler(profiler) {}

    // Print metrics over ImGui
    void printTimes();

  private:
    void printSectionTimes(const Profiler::Section& section);
    const Profiler& mProfiler;
  };
}
