#pragma once

#include <filesystem>

namespace selwonk {
// Platform-specific functions
// Currently, only Unix-like platforms are supported
class Platform {
public:
  static std::filesystem::path getExePath();

private:
  Platform() = delete;
};
} // namespace selwonk
