#include <gtest/gtest.h>

#include <vncore/ringbuffer.hpp>

namespace selwonk::core::test {

const static constexpr size_t Count = 10;
TEST(RingBuffer, ringbuffer) {
  RingBuffer<int, Count> buffer;

  // Reading average before writing should be ok
  ASSERT_EQ(buffer.average(), 0);

  // If the buffer is not filled, only average known samples
  buffer.record(100);
  ASSERT_EQ(buffer.average(), 100);
  buffer.record(200);
  ASSERT_EQ(buffer.average(), 150);

  // After filling, old samples should be discarded first in first out
  for (int i = 0; i < Count - 2; i++) {
    buffer.record(200);
  }
  // All samples in place
  auto sum = (200 * Count) + (100 - 200);
  ASSERT_EQ(buffer.average(), sum / Count);
  buffer.record(200);
  ASSERT_EQ(buffer.average(), 200);

  // Filling some more should only account for new samples
  for (int i = 0; i < Count; i++) {
    buffer.record(1000);
  }
  ASSERT_EQ(buffer.average(), 1000);
}
}