#pragma once

#include <array>
#include <algorithm>

namespace selwonk::core {
// Ring buffer that tracks a metric over time
template <typename T, size_t Count> class RingBuffer {
public:
  void record(T sample) {
    mSamples[mIndex] = sample;
    mKnownSamples++;
    mIndex = (mIndex + 1) % mSamples.size();
  }

  // Get the mean of the metric over the last Count samples
  // Returns zero if nothing has been recorded yet
  T average() const {
    if (mKnownSamples == 0) return T{};
    auto currentRecordings = std::min(mSamples.size(), mKnownSamples);

    T sum{};
    for (int i = 0; i < currentRecordings; i++) {
      sum += mSamples[i];
    }

    return sum / currentRecordings;
  }

private:
  size_t mIndex = 0;
  size_t mKnownSamples = 0;
  std::array<T, Count> mSamples;
};
} // namespace selwonk::core
