#pragma once

#include <filesystem>

namespace selwonk::core {
// Platform-specific functions
// Currently, only Unix-like platforms are supported
class Platform {
public:
  static std::filesystem::path getExePath();

  // Get process memory usage, in bytes
  static size_t getMemoryUsage();

private:
  Platform() = delete;
};
} // namespace selwonk::core
