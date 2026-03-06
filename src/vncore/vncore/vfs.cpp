#include "vfs.hpp"
#include <cstdio>

namespace selwonk {
const std::filesystem::path Vfs::Shaders = "shaders";
const std::filesystem::path Vfs::Meshes = "meshes";

std::ifstream Vfs::open(Path& path) {
  for (auto& provider : mProviders) {
    if (auto file = provider->open(path)) {
      return file;
    }
  }
  throw std::runtime_error("File not found");
}

void Vfs::readfull(Path path, std::vector<std::byte>& buffer) {
  static_assert(sizeof(std::byte) == sizeof(char),
                "char must be 1 byte, otherwise we'll have to rethink this");
  auto file = open(path);
  file.seekg(0, std::ios::end);
  buffer.resize(file.tellg());
  file.seekg(0, std::ios::beg);
  file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
}

} // namespace selwonk
