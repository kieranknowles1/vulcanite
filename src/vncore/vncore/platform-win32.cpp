#include "platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>
#include <codecvt>

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

void Platform::setThreadName(std::thread& thread, const char* name)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  auto wstring = converter.from_bytes(name);

  SetThreadDescription(thread.native_handle(), wstring.c_str());
}

} // namespace selwonk::core
