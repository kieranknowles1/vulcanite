#pragma once

#include <chrono>
#include <string_view>
#include <vector>

#include "ringbuffer.hpp"
#include "singleton.hpp"

namespace selwonk::core {
// Basic profiler. Assumes the same sections are present every frame
class Profiler : public Singleton<Profiler> {
public:
  struct Metrics {
    int totalRenderable = 0;
    int drawnRenderable = 0;
    int transparentRenderable = 0;
  };

  Profiler();

  // Push a new section to the tree, must be followed by exactly one popSection
  // or siblingSection
  void pushSection(std::string_view name);
  void popSection();
  // Start a new section as a sibling to the current one, utility to pop then
  // immediately push
  void siblingSection(std::string_view name) {
    popSection();
    pushSection(name);
  }

  Metrics& getExtraMetrics() { return mExtraMetrics; }

  void beginFrame();
  void endFrame();
  // Print metrics over ImGui
  void printTimes();

private:
  using Clock = std::chrono::high_resolution_clock;

  const static constexpr int Samples = 128;
  struct Section {
    Section(std::string_view name, Section* parent)
        : mName(name), mParent(parent) {}

    std::string mName;
    Section* mParent = nullptr;
    std::vector<std::unique_ptr<Section>> mChildren;
    RingBuffer<Clock::duration, Samples> mSamples;
    Clock::time_point mLastStart;

    Section* getOrAdd(std::string_view name);

    void begin() { mLastStart = Clock::now(); }
    void end() { mSamples.record(Clock::now() - mLastStart); }

    float timeMs() const {
      auto us = std::chrono::duration_cast<std::chrono::microseconds>(
          mSamples.average());
      return static_cast<float>(us.count()) / 1000.0f;
    }
  };

  void printSectionTimes(const Section& section);

  Section mRootSection = Section("Root", nullptr);
  Section* mCurrentSection = &mRootSection;
  Metrics mExtraMetrics;
};
} // namespace selwonk::core
