#include "platform.hpp"

namespace selwonk::core {
std::filesystem::path Platform::getExePath() {
  return "/"; // TODO
}

size_t Platform::getMemoryUsage() {
  return 0; // TODO
}

void Platform::setThreadName(std::thread& thread, const char* name) {
  // TODO
}

} // namespace selwonk::core
