#include "util.hpp"

#include <random>
#include <fstream>
#include <cassert>

namespace selwonk::test::util {

std::string randomString(std::size_t length) {
  static const std::string chars =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";

  thread_local std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<> dist(0, chars.size() - 1);

  std::string result;
  result.reserve(length);

  for (std::size_t i = 0; i < length; ++i) {
    result += chars[dist(rng)];
  }

  return result;
}

std::filesystem::path tempDir() {
  return std::filesystem::temp_directory_path();
}

std::filesystem::path writeDummyFile(const std::vector<char>& data) {
  auto name = tempDir() / randomString(8);

  std::ofstream of(name, std::ofstream::binary);
  assert(of.is_open());
  of.write(data.data(), data.size());
  assert(of);
  return name;
}

}