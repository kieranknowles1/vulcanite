#include "vfs.hpp"
#include <cstdio>

namespace selwonk::core {

Vfs::FilePtr Vfs::get(Path& path) {
  for (auto& provider : mProviders) {
    if (auto file = provider->open(path)) {
      return file;
    }
  }
  throw std::runtime_error("File not found");
}

void Vfs::File::readfull(std::vector<std::byte>& buffer) {
  static_assert(sizeof(std::byte) == sizeof(char), "wtf is this architecture?");
  auto file = open();
  file.seekg(0, std::ios::end);
  buffer.resize(file.tellg());
  file.seekg(0, std::ios::beg);
  file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
}

} // namespace selwonk::core
