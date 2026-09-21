#pragma once

#include <vncore/profiler.hpp>

#include "element.hpp"

namespace selwonk::ui {
  class ProfilerUi final : public Element {
  public:
    ProfilerUi(const core::Profiler& profiler)
      : mProfiler(profiler) {}
    ~ProfilerUi() = default;

    const char* name() const { return "Profiler"; }
    // Print metrics over ImGui
    void drawImpl() override;

  private:
    void printSectionTimes(const core::Profiler::Section& section);
    const core::Profiler& mProfiler;
  };
}
