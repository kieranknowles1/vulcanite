#include "platform.hpp"

#include <malloc.h>

namespace selwonk::core {
std::filesystem::path Platform::getExePath() {
  return "/"; // We mount files at /
}

size_t Platform::getMemoryUsage() {
  // TODO: Is this accurate? Should we use this on Linux?
  auto info = mallinfo();
  return info.uordblks;
}

void Platform::setThreadName(std::thread& thread, const char* name) {
  // TODO
}

} // namespace selwonk::core
