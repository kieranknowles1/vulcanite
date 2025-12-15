#include "vfs.hpp"
#include <cstdio>
#include <unistd.h>

namespace selwonk {
const std::filesystem::path Vfs::Shaders = "shaders";

std::ifstream Vfs::open(const std::filesystem::path &path) {
  for (auto &provider : mProviders) {
    if (auto file = provider->open(path)) {
      return file;
    }
  }
  throw std::runtime_error("File not found");
}

std::filesystem::path Vfs::getExePath() {
  char path[FILENAME_MAX];
  int bytes = readlink("/proc/self/exe", path, FILENAME_MAX);
  if (bytes == -1 || bytes == FILENAME_MAX) {
    throw std::runtime_error("Failed to get executable path");
  }
  return std::filesystem::path(path);
}

} // namespace selwonk
