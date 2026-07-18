#include "platform.hpp"

#include <pthread.h>
#include <stdexcept>
#include <sys/resource.h>
#include <unistd.h>

namespace selwonk::core {
std::filesystem::path Platform::getExePath() {
  char path[FILENAME_MAX];
  int bytes = readlink("/proc/self/exe", path, FILENAME_MAX);
  if (bytes == -1 || bytes == FILENAME_MAX) {
    throw std::runtime_error("Failed to get executable path");
  }
  path[bytes] = 0; // Readlink does not append null terminator
  return std::filesystem::path(path);
}

size_t Platform::getMemoryUsage() {
  rusage usage;
  getrusage(RUSAGE_SELF, &usage);
  return usage.ru_maxrss * 1024;
}

void Platform::setThreadName(std::thread& thread, const char* name) {
  auto status = pthread_setname_np(thread.native_handle(), name);
  if (status != 0) {
    throw std::runtime_error("Failed to set thread name");
  }
}

} // namespace selwonk::core
