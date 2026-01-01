#pragma once

#include <array>
#include <stdexcept>
#include <string_view>

#include <fmt/format.h>

namespace selwonk::core {

// A fixed-size string class that never allocates memory
// Holds up to Capacity characters
// Requires sizeof(CapacityType) + Capacity storage and size
template <typename CapacityType, CapacityType Capacity> class FixedString {
public:
  FixedString() : mSize(0) {}
  FixedString(std::string_view str) {
    if (str.size() > Capacity) {
      throw std::invalid_argument("String too long");
    }
    std::copy(str.begin(), str.end(), mData.begin());
    mSize = str.size();
  }
  FixedString(const std::string& str) : FixedString(std::string_view(str)) {}

  std::string_view view() const {
    return std::string_view(mData.data(), mSize);
  }

private:
  CapacityType mSize;
  std::array<char, Capacity> mData;
};

} // namespace selwonk::core

template <typename CapacityType, CapacityType Capacity>
struct fmt::formatter<selwonk::core::FixedString<CapacityType, Capacity>>
    : formatter<std::string_view> {
  auto format(const selwonk::core::FixedString<CapacityType, Capacity>& str,
              format_context& ctx) const {
    return formatter<std::string_view>::format(str.view(), ctx);
  }
};
