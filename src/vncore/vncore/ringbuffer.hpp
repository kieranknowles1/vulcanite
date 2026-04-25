#pragma once

#include <array>
#include <memory.h>

namespace selwonk::core {
// Ring buffer that tracks a metric over time
template <typename T, size_t Count> class RingBuffer {
public:
  void record(T sample) {
    mSamples[mIndex] = sample;
    mKnownSamples++;
    mIndex = (mIndex + 1) % mSamples.size();
  }

  T average() const {
    T sum{};
    for (int i = 0; i < std::min(mSamples.size(), mKnownSamples); i++) {
      sum += mSamples[i];
    }

    return sum / Count;
  }

private:
  size_t mIndex = 0;
  size_t mKnownSamples = 0;
  std::array<T, Count> mSamples;
};
} // namespace selwonk::core
