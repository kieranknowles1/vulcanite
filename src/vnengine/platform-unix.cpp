#include "platform.hpp"

#include <unistd.h>

namespace selwonk {
std::filesystem::path Platform::getExePath() {
  char path[FILENAME_MAX];
  int bytes = readlink("/proc/self/exe", path, FILENAME_MAX);
  if (bytes == -1 || bytes == FILENAME_MAX) {
    throw std::runtime_error("Failed to get executable path");
  }
  return std::filesystem::path(path);
}

} // namespace selwonk
