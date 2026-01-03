#include "profiler.hpp"

#include <fmt/base.h>
#include <imgui.h>

namespace selwonk::core {
void Profiler::beginFrame() {
  mNextSectionIndex = 0;
  mLastSectionEnd = Clock::now();
}

Profiler::Clock::duration Profiler::getElapsed() {
  auto now = Clock::now();
  auto elapsed = now - mLastSectionEnd;
  mLastSectionEnd = now;
  return elapsed;
}

// Record timing for the last section
void Profiler::endFrame() { mMetrics.back().mSamples.record(getElapsed()); }

void Profiler::printTimes() {
  if (ImGui::Begin("Metrics")) {
    Clock::duration total{};
    for (auto& section : mMetrics) {
      auto avg = section.mSamples.average();
      auto ms = std::chrono::duration_cast<std::chrono::microseconds>(avg);
      total += avg;
      ImGui::LabelText(section.mName.c_str(), "%.3fms",
                       static_cast<float>(ms.count()) / 1000.0f);
    }

    auto ms = std::chrono::duration_cast<std::chrono::microseconds>(total);
    ImGui::LabelText("Total/Target", "%.3fms/%.3fms",
                     static_cast<float>(ms.count() / 1000.0f),
                     1000.0f / 144.0f);
  }
  ImGui::End();
}

void Profiler::startSection(std::string_view name) {
  if (mMetrics.size() <= mNextSectionIndex) {
    mMetrics.emplace_back();
    mMetrics.back().mName = name;
  }

  // Record the previous section's time, unless we are the first section
  if (mNextSectionIndex > 0) {
    mMetrics[mNextSectionIndex - 1].mSamples.record(getElapsed());
  }
  mNextSectionIndex++;
}
} // namespace selwonk::core
