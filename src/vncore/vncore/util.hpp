#pragma once

#include <array>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <type_traits>

namespace selwonk::core::util {
inline std::string formatFilesize(size_t bytes) {
  std::array<std::string_view, 4> suffixes = {"B", "KB", "MB", "GB"};
  int index = 0;
  double size = bytes;
  while (size >= 1024 && index < suffixes.size()) {
    size /= 1024.0;
    index++;
  }
  std::ostringstream out;
  out << std::fixed << std::setprecision(2) << size << suffixes[index];
  return out.str();
}

template <typename T> constexpr bool hasFlag(T value, T flag) {
  using Base = std::underlying_type_t<T>;
  return (static_cast<Base>(value) & static_cast<Base>(flag)) != 0;
}

template <typename T, typename... Rest>
constexpr T combineFlags(T first, Rest... rest) {
  static_assert((std::is_same_v<T, Rest> && ...));
  using Base = std::underlying_type_t<T>;
  return static_cast<T>(
      (static_cast<Base>(first) | ... | static_cast<Base>(rest)));
}
} // namespace selwonk::core::util
