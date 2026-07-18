#pragma once

#include <filesystem>
#include <thread>

namespace selwonk::core {
// Platform-specific functions
// Currently, only Unix-like platforms are supported
class Platform {
public:
  static std::filesystem::path getExePath();

  // Get process memory usage, in bytes
  static size_t getMemoryUsage();

  // Set human-readable name for use in debuggers
  static void setThreadName(std::thread& thread, const char* name);

private:
  Platform() = delete;
};
} // namespace selwonk::core
