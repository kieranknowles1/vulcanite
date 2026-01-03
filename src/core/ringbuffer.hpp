#pragma once

#include <array>
#include <memory.h>

namespace selwonk::core {
// Ring buffer that tracks a metric over time
template <typename T, size_t Count> class RingBuffer {
public:
  void record(T sample) {
    mSamples[mIndex] = sample;
    mIndex++;
    if (mIndex >= mSamples.size())
      mIndex = 0;
  }

  T average() {
    T sum{};
    for (auto& s : mSamples) {
      sum += s;
    }
    return sum / Count;
  }

private:
  size_t mIndex = 0;
  std::array<T, Count> mSamples;
};
} // namespace selwonk::core
