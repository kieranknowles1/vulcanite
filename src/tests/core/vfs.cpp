#include <gtest/gtest.h>

#include <random>

#include <vncore/vfs.hpp>

namespace selwonk::core::test {

class VfsFixture : public testing::Test {
private:
  std::filesystem::path randomTmpPath() {
    static const std::string_view chars = "abcdefghijklmnopqrstuvwxyz"
                                          "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                          "0123456789";
    thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<> dist(0, chars.size() - 1);

    std::string name = "vulcanite-test-";
    for (std::size_t i = 0; i < TestDirLength; ++i) {
      name += chars[dist(rng)];
    }
    return std::filesystem::temp_directory_path() / name;
  }

  Vfs createVfs(std::filesystem::path baseDir) {
    Vfs::Providers providers;
    providers.emplace_back(std::make_unique<Vfs::FilesystemProvider>(baseDir));
    return Vfs(std::move(providers));
  }

protected:
  const static constexpr int TestDirLength = 10;
  const std::filesystem::path mDirectory;
  const Vfs mVfs;

  VfsFixture() : mDirectory(randomTmpPath()), mVfs(createVfs(mDirectory)) {
    std::filesystem::create_directory(mDirectory);
  }
  ~VfsFixture() override { std::filesystem::remove_all(mDirectory); }

  void writeFile(std::string_view name, std::span<char> contents) {
    std::ofstream of(mDirectory / name, std::ofstream::binary);
    if (!of.is_open()) {
      throw std::runtime_error("Failed to open file");
    }
    of.write(contents.data(), contents.size());
    if (!of.good()) {
      throw std::runtime_error("Failed to write file");
    }
  }
};

TEST_F(VfsFixture, ReadDoesNotModifyLineEndings) {
  std::string_view fileName = "dummy.txt";
  std::vector<char> data{'\r', ' ', '\n', ' ', '\n', '\r', '\0'};

  writeFile(fileName, data);

  std::vector<char> out;
  mVfs.get(fileName)->readfull(out);

  ASSERT_EQ(data, out);
}

} // namespace selwonk::core::test
