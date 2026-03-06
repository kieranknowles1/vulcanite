#pragma once

#include <cassert>
#include <cstdint>
#include <limits>

namespace selwonk::vulkan {
// TODO: Ref counting
class Handle {
public:
  using Backing = uint32_t;
  const static constexpr Backing InvalidValue =
      std::numeric_limits<Backing>::max();

  explicit Handle(Backing v) : mValue(v) {}
  Handle() : mValue(InvalidValue) {}
  constexpr Backing value() const {
    assert(valid());
    return mValue;
  }
  constexpr bool valid() const { return mValue != InvalidValue; }

private:
  Backing mValue;
};
} // namespace selwonk::vulkan
