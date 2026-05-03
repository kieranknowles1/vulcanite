#include "platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>

namespace selwonk::core {
std::filesystem::path Platform::getExePath() {
  const static constexpr size_t MaxPath = 4096;
  TCHAR path[MaxPath];
  auto len = GetModuleFileName(nullptr, path, MaxPath);
  if (len == -1 || len == MaxPath) {
    throw std::runtime_error("Failed to get executable path");
  }
  return std::filesystem::path(path);
}

size_t Platform::getMemoryUsage() {
  PROCESS_MEMORY_COUNTERS info;
  GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));

  return info.WorkingSetSize;
}

} // namespace selwonk::core
