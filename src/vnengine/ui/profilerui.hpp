#pragma once

#include <vncore/profiler.hpp>

namespace selwonk::ui {
  class ProfilerUi {
  public:
    ProfilerUi(const core::Profiler& profiler)
      : mProfiler(profiler) {}

    // Print metrics over ImGui
    void printTimes();

  private:
    void printSectionTimes(const core::Profiler::Section& section);
    const core::Profiler& mProfiler;
  };
}
